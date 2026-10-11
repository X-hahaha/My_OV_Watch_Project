#ifndef __APP_EVENT_H__
#define __APP_EVENT_H__

#define KEY_EVENT_EDGE         (1UL << 0)


#define APP_EVT_INIT_DONE      (1UL << 0)
#define APP_EVT_CHARGE         (1UL << 1)   // GDB 注入测试
#define APP_EVT_WAKE           (1UL << 2)   // 按键事件
#define APP_EVT_UART_HEALTH    (1UL << 3)   // UART 任务心跳
#define APP_EVT_SAMPLE_HEALTH  (1UL << 4)   // 采样任务心跳
#define APP_EVT_TIMER_PULSE    (1UL << 5)   // 1s Timer 事件

#endif


