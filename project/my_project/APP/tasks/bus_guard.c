#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "app_entry.h"
#include "app_diag.h"
#include "app_event.h" 

SemaphoreHandle_t bus_mutex = NULL;
volatile uint8_t in_bus = 0;


void bus_test_task(void *pvParameters)
{
    EventBits_t events = xEventGroupWaitBits(app_events, APP_EVT_INIT_DONE, pdFALSE, pdTRUE, pdMS_TO_TICKS(1000U));
    if((events & APP_EVT_INIT_DONE) == 0U)
    {
        Day06Fail(DAY06_FAILURE_INIT_TIMEOUT);
    }
    TickType_t hold_time = *(TickType_t *)pvParameters;
    while(1)
    {
        if(bus_mutex != NULL)
        {
            if((xSemaphoreTake(bus_mutex, pdMS_TO_TICKS(50)) == pdTRUE))
            {
                if(!in_bus)
                {
                    g_day06_diag_stats.mutex_enter++;
                    in_bus = 1;
                    vTaskDelay(pdMS_TO_TICKS(hold_time));
                    in_bus = 0;
                    xSemaphoreGive(bus_mutex);
                    g_day06_diag_stats.mutex_exit++;
                    taskYIELD();
                }
                else
                {
                    xSemaphoreGive(bus_mutex);
                }

            }
            else
            {
                g_day06_diag_stats.mutex_timeout++; 
            }
        }
    }
}
