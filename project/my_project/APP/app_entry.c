#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "app_event.h"
#include "app_diag.h"
#include "uart_rx_task.h"
#include "sample_task.h"
#include "bus_guard.h"
#include "timers.h"

TaskHandle_t key_task_handle = NULL;
TaskHandle_t init_task_handle = NULL;
TaskHandle_t sample_producer_task_handle = NULL;
TaskHandle_t sample_consumer_task_handle = NULL;
TaskHandle_t uart_rx_task_handle = NULL;
TaskHandle_t bus_test_taska_handle = NULL;
TaskHandle_t bus_test_taskb_handle = NULL;
TaskHandle_t status_task_handle = NULL;
TimerHandle_t slow_timer = NULL;

/* 事件组 */
EventGroupHandle_t app_events = NULL;
/* 流缓冲 */
StreamBufferHandle_t uart_stream = NULL;

volatile uint8_t app_isr_ready = 0;


/*
 * 初始化任务 —— 上电后运行一次，完成按键中断的初始化配置后自删除。
 *
 * 执行流程：
 *  1. 关闭 ISR 就绪标志，防止中断服务函数在初始化期间调用 FreeRTOS API；
 *  2. 调用 HAL EXTI 中断处理函数，清除上电过程中可能产生的按键挂起中断；
 *  3. 重新置位 ISR 就绪标志，允许后续中断安全使用 FreeRTOS 通知；
 *  4. 使能按键对应的 NVIC 中断线（EXTI9_5）；
 *  5. 删除本任务，释放栈空间。
 */
static void init_task(void *pvParameters)
{
    (void) pvParameters;
    app_isr_ready = 0;
    HAL_GPIO_EXTI_IRQHandler(KEY1_Pin);
    app_isr_ready = 1;
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    xEventGroupSetBits(app_events, APP_EVT_INIT_DONE);
    xTimerStart(slow_timer, pdMS_TO_TICKS(100U));
    vTaskDelete(NULL);

}

static void key_task(void *pvParameters)
{
    (void) pvParameters;
    uint32_t events;
    uint8_t released = 1;

    for(;;)
    {
        if(xTaskNotifyWait(KEY_EVENT_EDGE, UINT32_MAX, &events, portMAX_DELAY) == pdTRUE)
        {
            if(events & KEY_EVENT_EDGE)
            {
                vTaskDelay(pdMS_TO_TICKS(20));
                if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
                {
                    if(released)
                    {
                        g_day06_diag_stats.key_valid_press++;
                        released = 0;
                        xEventGroupSetBits(app_events, APP_EVT_WAKE);
                    }
                    while(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
                    {
                        vTaskDelay(pdMS_TO_TICKS(5));
                    }
                    g_day06_diag_stats.key_valid_release++;
                    released = 1;
                }
            }
        }
    }
    
}



static void SlowTimerCallback(TimerHandle_t timer)
{
    (void)timer;
    g_day06_diag_stats.timer_callback_count++;
    xEventGroupSetBits(app_events, APP_EVT_TIMER_PULSE);
}

static void status_task(void *pvParameters)
{
    // 等 INIT_DONE
    EventBits_t bits = xEventGroupWaitBits(app_events, APP_EVT_INIT_DONE, pdFALSE, pdTRUE, 1000);
    if((bits & APP_EVT_INIT_DONE) == 0)
    {
        Day06Fail(DAY06_FAILURE_INIT_TIMEOUT); 
    }
    TickType_t last_health_check = xTaskGetTickCount();
    
    while(1)
    {
        // 等事件（TIMER_PULSE 或 WAKE），超时 1 秒做健康检查
        EventBits_t active = xEventGroupWaitBits(
            app_events,
            APP_EVT_TIMER_PULSE | APP_EVT_WAKE,
            pdTRUE,   // 收到后清除这两个一次性位
            pdFALSE,  // ANY 即可
            pdMS_TO_TICKS(1000));
        
        if(active & APP_EVT_TIMER_PULSE) g_day06_diag_stats.evt_timer_count++;
        if(active & APP_EVT_WAKE) g_day06_diag_stats.evt_wake_count++;
        
        // 每秒做一次健康检查
        if(xTaskGetTickCount() - last_health_check >= pdMS_TO_TICKS(1000))
        {
            last_health_check = xTaskGetTickCount();
            EventBits_t health = xEventGroupWaitBits(
                app_events,
                APP_EVT_UART_HEALTH | APP_EVT_SAMPLE_HEALTH,
                pdTRUE,    // 清除健康位
                pdTRUE,    // 两个都要到
                0);        // 不等待，立刻检查
            if((health & (APP_EVT_UART_HEALTH | APP_EVT_SAMPLE_HEALTH)) != 
                (APP_EVT_UART_HEALTH | APP_EVT_SAMPLE_HEALTH))
            {
                g_day06_diag_stats.evt_health_timeout++;
            }
        }
    }
}


void App_Start(void)
{
    Day06Stats_Init();
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);

    static TickType_t hold_time_a = pdMS_TO_TICKS(20U);
    static TickType_t hold_time_b = pdMS_TO_TICKS(100U);

    app_events = xEventGroupCreate();
    if(app_events == NULL)
    {
        Day06Fail(DAY06_FAILURE_EVENT_CREATE);
    }
    uart_stream = xStreamBufferCreate(512U, 1U);
    if (uart_stream == NULL) 
    {
        Day06Fail(DAY06_FAILURE_STREAM_CREATE);
    }
    sample_queue = xQueueCreate(10U, sizeof(Sample_t));
    if(sample_queue == NULL)
    {
        Day06Fail(DAY06_FAILURE_QUEUE_CREATE);
    }
    bus_mutex = xSemaphoreCreateMutex();
    if(bus_mutex == NULL)
    {
        Day06Fail(DAY06_FAILURE_MUTEX_CREATE);
    }
    slow_timer = xTimerCreate("slow", pdMS_TO_TICKS(1000U), pdTRUE, NULL, SlowTimerCallback);
    if(slow_timer == NULL) 
    {
        Day06Fail(DAY06_FAILURE_TIMER_CREATE);
    }
    

    if(xTaskCreate(init_task, "init_task", 256, NULL, 4, &init_task_handle) != pdPASS)
    {
        Day06Fail(DAY06_FAILURE_TASK_CREATE);
    }
    if(xTaskCreate(key_task, "key_task", 256, NULL, 3, &key_task_handle) != pdPASS)
    {
        Day06Fail(DAY06_FAILURE_TASK_CREATE);
    }
    if (xTaskCreate(uart_rx_task, "uart_rx", 512U, NULL, 4, &uart_rx_task_handle) != pdPASS)
    {
        Day06Fail(DAY06_FAILURE_TASK_CREATE);
    }
    if (xTaskCreate(sample_producer_task, "sample_producer", 256U, NULL, 2, &sample_producer_task_handle) != pdPASS)
    {
        Day06Fail(DAY06_FAILURE_TASK_CREATE);
    }
    if (xTaskCreate(sample_consumer_task, "sample_consumer", 256U, NULL, 2, &sample_consumer_task_handle) != pdPASS)
    {
        Day06Fail(DAY06_FAILURE_TASK_CREATE);
    }
    if (xTaskCreate(bus_test_task, "bus_test_taska", 256U, &hold_time_a, 1, &bus_test_taska_handle) != pdPASS)    
    {   
        Day06Fail(DAY06_FAILURE_TASK_CREATE);
    }
    if (xTaskCreate(bus_test_task, "bus_test_taskb", 256U, &hold_time_b, 1, &bus_test_taskb_handle) != pdPASS)    
    {   
        Day06Fail(DAY06_FAILURE_TASK_CREATE);
    }
    if (xTaskCreate(status_task, "status_task", 256U, NULL, 1, &status_task_handle) != pdPASS)
    {
        Day06Fail(DAY06_FAILURE_TASK_CREATE);
    }



    HAL_GPIO_WritePin(BLE_EN_GPIO_Port, BLE_EN_Pin, GPIO_PIN_SET);  // PA8 拉高，给 KT6328A1 上电
    vTaskStartScheduler();
    Day06Fail(DAY06_FAILURE_SCHEDULRE_RETURNED);
}