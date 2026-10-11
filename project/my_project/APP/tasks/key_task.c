#include <stdint.h>
#include "main.h"
#include "app_entry.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app_event.h"
#include "app_diag.h"
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY1_Pin && app_isr_ready == 1)
    {
        g_day06_diag_stats.key_irq_count++;
        BaseType_t highTaskWoken = pdFALSE;
        BaseType_t rc = xTaskNotifyFromISR(key_task_handle, KEY_EVENT_EDGE, eSetBits, &highTaskWoken);
        if (rc != pdPASS)
        {
            g_day06_diag_stats.key_irq_fail++;
        }
        portYIELD_FROM_ISR(highTaskWoken);

    }
}
