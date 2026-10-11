#ifndef __UART_RX_TASK_H__
#define __UART_RX_TASK_H__

#include "stream_buffer.h"

extern StreamBufferHandle_t uart_stream;

void uart_rx_task(void *pvParameters);

#endif
