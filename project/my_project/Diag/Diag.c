#include "Diag.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lvgl.h"
volatile stats_t g_diag_stats;

void FAIL(DiagFail_t fail)
{
    g_diag_stats.fail = fail;
    configASSERT(0);
}

void vApplicationIdleHook(void)
{
    g_diag_stats.idle_hook_count++;
}


void vApplicationTickHook(void)
{
    g_diag_stats.tick_hook_count++;
    lv_tick_inc(1);
}
void vApplicationMallocFailedHook(void)
{
    FAIL(DIAG_FAIL_MallocFailed);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    FAIL(DIAG_FAIL_StackOverflow);
}


