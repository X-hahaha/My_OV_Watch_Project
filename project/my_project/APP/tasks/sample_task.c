#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "app_diag.h"
#include "app_entry.h"
#include "sample_task.h"
#include "string.h"
#include "app_event.h"

QueueHandle_t sample_queue = NULL;

void sample_producer_task(void *pvParameters)
{

    (void) pvParameters;
    TickType_t last_tick = xTaskGetTickCount();
    struct Sample sample_send;
/* 等待初始化完成*/
    EventBits_t events = xEventGroupWaitBits(app_events, APP_EVT_INIT_DONE, pdFALSE, pdTRUE, pdMS_TO_TICKS(1000U));
    if((events & APP_EVT_INIT_DONE) == 0U)
    {
        Day06Fail(DAY06_FAILURE_INIT_TIMEOUT);
    }
/* 复位结构体 */
    memset(&sample_send, 0, sizeof(struct Sample));
    while(1)
    {
        if(sample_queue != NULL)
        {
            
            g_day06_diag_stats.q_send_attempt++;
            if(xQueueSend(sample_queue, &sample_send, pdMS_TO_TICKS(50))== pdPASS)
            {
                g_day06_diag_stats.q_send_ok++;
            }
            else    
            {
                g_day06_diag_stats.q_send_fail++;
            }
            sample_send.seq++;
            sample_send.produced_tick = xTaskGetTickCount(); 
            sample_send.value_milli++;
        }
        vTaskDelayUntil(&last_tick, pdMS_TO_TICKS(20));
    }
}

void sample_consumer_task(void *pvParameters)
{
    (void) pvParameters;
    uint32_t expect_seq = 0;
    struct Sample sample_recv;

    EventBits_t events = xEventGroupWaitBits(app_events, APP_EVT_INIT_DONE, pdFALSE, pdTRUE, pdMS_TO_TICKS(1000U));
    if((events & APP_EVT_INIT_DONE) == 0U)
    {
        Day06Fail(DAY06_FAILURE_INIT_TIMEOUT);
    }

    memset(&sample_recv, 0, sizeof(struct Sample));

    while(1)
    {
        if(sample_queue != NULL)
        {

            xEventGroupSetBits(app_events, APP_EVT_SAMPLE_HEALTH);
            if(xQueueReceive(sample_queue, &sample_recv, pdMS_TO_TICKS(100)) == pdPASS)
            {
                g_day06_diag_stats.q_recv_count++;
                if(sample_recv.seq == expect_seq && sample_recv.value_milli == sample_recv.seq)
                {
                    expect_seq = sample_recv.seq + 1;
                }
                else
                {
                    g_day06_diag_stats.q_seq_err++;
                }
            }
            else
            {
                g_day06_diag_stats.q_recv_timeout++;
            }
        }
    }
}
