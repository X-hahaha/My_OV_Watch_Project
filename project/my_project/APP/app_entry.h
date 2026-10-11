#ifndef __APP_ENTRY_H__
#define __APP_ENTRY_H__
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "stream_buffer.h"

extern volatile uint8_t app_isr_ready;
extern TaskHandle_t key_task_handle;
extern TaskHandle_t init_task_handle;
extern EventGroupHandle_t app_events;
extern StreamBufferHandle_t uart_stream;

void App_Start(void);
#endif

