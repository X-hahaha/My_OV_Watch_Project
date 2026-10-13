#ifndef __DIAG_H__  
#define __DIAG_H__  

#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

#define LOG(fmt, ...)  printf(fmt "\r\n", ##__VA_ARGS__)

typedef enum
{
    DIAG_FAIL_Boot,
    DIAG_FAIL_TaskCreate,
    DIAG_FAIL_SemaphoreCreate,
    DIAG_FAIL_SchedulerReturn,
    DIAG_FAIL_MallocFailed,
    DIAG_FAIL_StackOverflow,
    DIAG_FAIL_EEPROMROM_Check
}DiagFail_t;

typedef struct
{
    DiagFail_t fail;
    uint32_t idle_hook_count;
    uint32_t tick_hook_count;

}stats_t;

extern volatile stats_t g_diag_stats;

void FAIL(DiagFail_t fail);

void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);

#endif
