#include "FreeRTOS.h"
#include "task.h"
#include "day05_rtos_demo.h"
#include "main.h"

#define Malloc_Failed_Fault           0U
#define Stack_Overflow_Fault          0U

#if (Stack_Overflow_Fault == 1)
#define Low_Task_Stack_Word          64U
#else
#define Low_Task_Stack_Word          256U   
#endif 

#define High_Task_Stack_Word         256U

#define Low_Task_Priority            (tskIDLE_PRIORITY + 1U)
#define High_Task_Priority           (tskIDLE_PRIORITY + 3U)

static TaskHandle_t low_task_handle = NULL;
static TaskHandle_t high_task_handle = NULL;
Day05RtosStats g_day05_rtos_stats = {0};


static void LowPriorityTask(void *pvParameters);
static void HighPriorityTask(void *pvParameters);
static void Day05Fail(Day05Failure failure);

void Day05Rtos_Start(void)
{
    BaseType_t result;
    result = xTaskCreate(LowPriorityTask, 
                        "LowTask", 
                        Low_Task_Stack_Word , 
                        NULL, 
                        Low_Task_Priority, 
                        &low_task_handle);
    if(result != pdPASS)
    {
        Day05Fail(DAY05_FAILURE_LOW_TASK_CREATE);
    }
    result = xTaskCreate(HighPriorityTask, 
                        "HighTask", 
                        High_Task_Stack_Word , 
                        NULL, 
                        High_Task_Priority, 
                        &high_task_handle);
    if(result != pdPASS)
    {
        Day05Fail(DAY05_FAILURE_HIGH_TASK_CREATE); 
    }
    g_day05_rtos_stats.free_heap_bytes = (uint32_t)xPortGetFreeHeapSize();
    g_day05_rtos_stats.minimum_ever_free_heap_bytes = (uint32_t)xPortGetMinimumEverFreeHeapSize();
/* -------------launch Malloc Failed Hook----------------*/
#if (Malloc_Failed_Fault == 1)
    void *test = pvPortMalloc(configTOTAL_HEAP_SIZE + 1U);
    (void)test;
#endif
/*-------------------------------------------------------*/
    vTaskStartScheduler();
    Day05Fail(DAY05_FAILURE_SCHEDULER_RETURNED); 
}


void LowPriorityTask(void *pvParameters)
{
    TickType_t last_tick = xTaskGetTickCount();
    uint32_t start_ms;
    (void)pvParameters;


    while(1)
    {
        #if (Stack_Overflow_Fault == 1)
            volatile uint8_t stack_test[256];
            stack_test[0] = 1U;
            taskYIELD();
            // 切换任务时内核会检查栈是否溢出，如果溢出则会调用栈溢出钩子函数
        #endif
        g_day05_rtos_stats.low_period_count++;
        HAL_GPIO_WritePin(LED_EN_GPIO_Port, LED_EN_Pin, GPIO_PIN_SET);
        start_ms = HAL_GetTick();
        while(HAL_GetTick() - start_ms < 20)
        {
            g_day05_rtos_stats.low_busy_count++;
        }
        HAL_GPIO_WritePin(LED_EN_GPIO_Port, LED_EN_Pin, GPIO_PIN_RESET);

        g_day05_rtos_stats.low_stack_min_words =
            (uint32_t)uxTaskGetStackHighWaterMark(NULL);
        g_day05_rtos_stats.free_heap_bytes =
            (uint32_t)xPortGetFreeHeapSize();
        g_day05_rtos_stats.minimum_ever_free_heap_bytes =
            (uint32_t)xPortGetMinimumEverFreeHeapSize();

        vTaskDelayUntil(&last_tick, pdMS_TO_TICKS(200));
    }
}


static void HighPriorityTask(void *argument)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    (void)argument;

    while(1)
    {
        g_day05_rtos_stats.high_period_count++;
        HAL_GPIO_TogglePin(WDI_GPIO_Port, WDI_Pin);
        g_day05_rtos_stats.high_stack_min_words =
            (uint32_t)uxTaskGetStackHighWaterMark(NULL);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(5U));
    }
}

void vApplicationIdleHook(void)
{
    g_day05_rtos_stats.idle_hook_count++;
}


void vApplicationTickHook(void)
{
    g_day05_rtos_stats.tick_hook_count++;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    Day05Fail(DAY05_FAILURE_STACK_OVERFLOW);
}

void vApplicationMallocFailedHook(void)
{
    Day05Fail(DAY05_FAILURE_MALLOC);
}

static void Day05Fail(Day05Failure failure)
{
    g_day05_rtos_stats.failure = failure;
    taskDISABLE_INTERRUPTS();
    while(1)
    {
        __NOP();
    }
}



