#ifndef __DAY05_RTOS_DEMO_H__
#define __DAY05_RTOS_DEMO_H__

#include "stdint.h"
#include "FreeRTOS.h"
typedef enum
{
    DAY05_FAILURE_NONE = 0,
    DAY05_FAILURE_LOW_TASK_CREATE,
    DAY05_FAILURE_HIGH_TASK_CREATE,
    DAY05_FAILURE_SCHEDULER_RETURNED,
    DAY05_FAILURE_STACK_OVERFLOW,
    DAY05_FAILURE_MALLOC,
    DAY05_FAILURE_ASSERT
    
}Day05Failure;

typedef struct
{
    volatile uint32_t low_period_count;
    volatile uint32_t low_busy_count;
    volatile uint32_t high_period_count;
    volatile uint32_t idle_hook_count;
    volatile uint32_t tick_hook_count;
    volatile uint32_t low_stack_min_words;
    volatile uint32_t high_stack_min_words;
    volatile uint32_t free_heap_bytes;
    volatile uint32_t minimum_ever_free_heap_bytes;
    volatile uint32_t assert_line;
    volatile Day05Failure failure;
    const char *volatile assert_file;
} Day05RtosStats;

extern Day05RtosStats g_day05_rtos_stats;

void Day05Rtos_Start(void);


#endif