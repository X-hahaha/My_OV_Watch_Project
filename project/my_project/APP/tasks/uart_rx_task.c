#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "app_diag.h"
#include "app_entry.h"
#include "app_event.h"
#include <string.h>


#define UART_DMA_RX_SIZE    128U
#define UART_CHUNK_SIZE     64U
#define FRAME_HEADER        0XA5U
#define FRAME_TAILER        0x5AU
#define FRAME_MAX_PAYLOAD   16U

typedef enum
{
    PS_SYNC,
    PS_LEN,
    PS_PAYLOAD,
    PS_CKSUM
}ParaseState_t;

typedef struct
{
    uint8_t header;
    uint8_t len;
    uint8_t payload[FRAME_MAX_PAYLOAD];
    uint8_t Tailer;
    uint8_t cksum;
}UART_Frame_t;

static uint8_t uart_dma_rx[UART_DMA_RX_SIZE];



volatile uint8_t uart_rx_err_pending = 0U;
volatile uint8_t uart_last_pos = 0U;
volatile uint8_t uart_rx_ready = 0U;    
volatile ParaseState_t UART_ParaseState = PS_SYNC;
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

static void UART_ParaseByte(uint8_t byte);
static void UartRecover(void);


static void UartPushFromISR(const uint8_t *data, size_t len, BaseType_t *pdHigherPriorityTaskWoken)
{
    if(len == 0) return ;
    size_t sent = xStreamBufferSendFromISR(uart_stream, data, len, pdHigherPriorityTaskWoken);
    g_day06_diag_stats.uart_copied += sent;
    g_day06_diag_stats.uart_dropped += (len - sent);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t len)
{
    
    BaseType_t higher_priority_task_woken = pdFALSE;

    if(huart != &huart1 || uart_rx_ready == 0U) return; 

    (void) len;
    uint16_t pos = UART_DMA_RX_SIZE - (uint16_t)__HAL_DMA_GET_COUNTER(huart->hdmarx);
    if(pos == UART_DMA_RX_SIZE) pos = 0;
    if(pos > uart_last_pos)
    {
        UartPushFromISR(&uart_dma_rx[uart_last_pos], pos - uart_last_pos, &higher_priority_task_woken);

    }
    else if(pos < uart_last_pos )
    {
        UartPushFromISR(&uart_dma_rx[uart_last_pos], UART_DMA_RX_SIZE - uart_last_pos, &higher_priority_task_woken);
        UartPushFromISR(&uart_dma_rx[0], pos, &higher_priority_task_woken); 
    }
    uart_last_pos = pos;
    portYIELD_FROM_ISR(higher_priority_task_woken);

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1) return;
    g_day06_diag_stats.uart_hal_err++;
    uart_rx_err_pending = 1U;
}

void uart_rx_task(void *pvParameters)
{
    (void) pvParameters;
    uint8_t chunk[UART_CHUNK_SIZE];

    EventBits_t events = xEventGroupWaitBits(app_events, APP_EVT_INIT_DONE, pdFALSE, pdTRUE, pdMS_TO_TICKS(1000U));

    if((events & APP_EVT_INIT_DONE) == 0U)
    {
        Day06Fail(DAY06_FAILURE_INIT_TIMEOUT);
    }
    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_dma_rx, sizeof(uart_dma_rx)) != HAL_OK) {
        Day06Fail(DAY06_FAILURE_UART_START);
    }
    uart_last_pos = 0U;
    uart_rx_ready = 1U;


    while(1)
    {
        xEventGroupSetBits(app_events, APP_EVT_UART_HEALTH);
        size_t n = xStreamBufferReceive(uart_stream, chunk, UART_CHUNK_SIZE, pdMS_TO_TICKS(100U));
        if(n > 0)
        {
            g_day06_diag_stats.uart_comsumed += (uint32_t)n;
            for(size_t i = 0; i < n; i++)
            {
                UART_ParaseByte(chunk[i]);
            }

        }
        if(uart_rx_err_pending != 0U)
        {
            UartRecover();
        }

        
    }
}


static void UART_ParaseByte(uint8_t byte)
{
    static UART_Frame_t UART_Frame;
    static uint8_t write_index = 0U;
    static uint8_t expected_seq = 0U;
    switch(UART_ParaseState)
    {
        case PS_SYNC:
            memset(&UART_Frame, 0, sizeof(UART_Frame_t));
            write_index = 0U;
            if(byte == FRAME_HEADER)
            {
                UART_Frame.header = byte;
                UART_ParaseState = PS_LEN;
            }
            break;
        case PS_LEN:
            if(byte == 0U || byte > FRAME_MAX_PAYLOAD)
            {
                g_day06_diag_stats.uart_check_err++;
                UART_ParaseState = PS_SYNC;
            }
            else
            {
                UART_Frame.cksum += byte;
                UART_Frame.len = byte;
                UART_ParaseState = PS_PAYLOAD;
            }
            break;
        case PS_PAYLOAD:
            if((byte == FRAME_TAILER) && (write_index == UART_Frame.len))
            {
                UART_Frame.Tailer = byte;
                UART_ParaseState = PS_CKSUM;
                
            }
            else if(write_index >= UART_Frame.len || write_index >= FRAME_MAX_PAYLOAD)
            {
                g_day06_diag_stats.uart_check_err++;
                UART_ParaseState = PS_SYNC;
            }
            else
            {
                UART_Frame.cksum += byte;
                UART_Frame.payload[write_index++] = byte;
            }
            break;
        case PS_CKSUM:
            if(byte == UART_Frame.cksum)
            {
                g_day06_diag_stats.uart_frames++;
                if(UART_Frame.payload[0] != expected_seq)
                {
                    g_day06_diag_stats.uart_seq_err++;
                }
                expected_seq = UART_Frame.payload[0] + 1U;

            }
            else
            {
                g_day06_diag_stats.uart_check_err++;
            }
            UART_ParaseState = PS_SYNC;
            break;
    }
}

static void UartRecover(void)
{
    uart_rx_ready = 0U;
    HAL_NVIC_DisableIRQ(DMA2_Stream5_IRQn);
    HAL_NVIC_DisableIRQ(USART1_IRQn);

    HAL_UART_AbortReceive(&huart1);

    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_NEFLAG(&huart1);
    __HAL_UART_CLEAR_FEFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, DMA_FLAG_TCIF1_5 | DMA_FLAG_HTIF1_5 | DMA_FLAG_TEIF1_5 | DMA_FLAG_DMEIF1_5);

/* 重置位置和解析器 */
    uart_last_pos = 0U;
    uart_rx_err_pending = 0U;
    UART_ParaseState = PS_SYNC;
    g_day06_diag_stats.uart_recoveries++;
    /* 重启接收 */
    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_dma_rx, sizeof(uart_dma_rx)) != HAL_OK) {
        Day06Fail(DAY06_FAILURE_UART_START);
    }
    uart_last_pos = 0U;
    uart_rx_ready = 1U;

    /* 6. 恢复 IRQ */
    HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

}

