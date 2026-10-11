#ifndef __APP_DIAG_H__
#define __APP_DIAG_H__
#include <stdint.h>

typedef enum
{
    DAY06_FAILURE_NONE = 0,
    DAY06_FAILURE_TASK_CREATE,
    DAY06_FAILURE_STREAM_CREATE,
    DAY06_FAILURE_QUEUE_CREATE,
    DAY06_FAILURE_MUTEX_CREATE,
    DAY06_FAILURE_EVENT_CREATE,
    DAY06_FAILURE_TIMER_CREATE,
    DAY06_FAILURE_UART_START,
    DAY06_FAILURE_INIT_TIMEOUT,
    DAY06_FAILURE_SCHEDULRE_RETURNED,
} Day06Failure;

typedef struct
{
    /*初始化与致命错误*/
    volatile Day06Failure failure;
    volatile uint32_t init_stage;
    volatile uint32_t fail_object_id;
    const char *volatile fail_object_name;

    /*按键实验*/
    volatile uint32_t key_irq_count;        /* EXTI ISR 进入次数 */
    volatile uint32_t key_irq_fail;         /* xTaskNotifyFromISR 返回非 pdPASS */
    volatile uint32_t key_valid_press;      /* 有效按下次数 */
    volatile uint32_t key_valid_release;    /* 有效释放次数 */

    /*串口+DMA实验*/
    volatile uint32_t uart_copied;
    volatile uint32_t uart_dropped;
    volatile uint32_t uart_comsumed;
    volatile uint32_t uart_frames;
    volatile uint32_t uart_seq_err;
    volatile uint32_t uart_check_err;
    volatile uint32_t uart_hal_err;
    volatile uint32_t uart_recoveries;

    /*模拟传感器队列实验*/
    volatile uint32_t q_send_attempt;   // 生产者尝试次数
    volatile uint32_t q_send_ok;        // 成功入队
    volatile uint32_t q_send_fail;      // 队列满/超时失败
    volatile uint32_t q_recv_count;     // 消费者收到条数
    volatile uint32_t q_recv_timeout;   // 消费者等超时次数
    volatile uint32_t q_seq_err;        // 消费者发现seq跳号
    
    /*总线互斥实验*/
    volatile uint32_t mutex_enter;
    volatile uint32_t mutex_exit;
    volatile uint32_t mutex_timeout;
    volatile uint32_t max_in_bus;

    /* 事件 + 软件定时器实验*/
    volatile uint32_t evt_timer_count;      // StatusTask 处理到的 TIMER_PULSE 次数
    volatile uint32_t evt_wake_count;       // StatusTask 处理到的按键唤醒次数
    volatile uint32_t evt_health_timeout;   // 健康检查超时次数
    volatile uint32_t timer_callback_count; // Timer 回调实际执行次数

} Day06DiagStats;

extern Day06DiagStats g_day06_diag_stats;

void Day06Fail(Day06Failure failure);
void Day06Stats_Init(void);

#endif

