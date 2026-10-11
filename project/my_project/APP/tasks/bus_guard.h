#ifndef __BUS_GUARD_H__ 
#define __BUS_GUARD_H__ 

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

extern SemaphoreHandle_t bus_mutex;

void bus_test_task(void *pvParameters);
#endif


