#include "app_diag.h"
#include "FreeRTOS.h"
#include "task.h"

Day06DiagStats g_day06_diag_stats = {0};

void Day06Stats_Init(void)
{
    g_day06_diag_stats.failure = DAY06_FAILURE_NONE;
    g_day06_diag_stats.init_stage = 0U;
}

void Day06Fail(Day06Failure failure)
{
    if(g_day06_diag_stats.failure == DAY06_FAILURE_NONE)
    {
        g_day06_diag_stats.failure = failure;
    }
    taskDISABLE_INTERRUPTS();
    for(;;)
    {
        __NOP();
    }
}



