#ifndef __SAMPLE_TASK_H__
#define __SAMPLE_TASK_H__

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern QueueHandle_t sample_queue;

struct Sample{
    uint32_t seq;
    TickType_t produced_tick;
    int32_t value_milli;
};

typedef struct Sample Sample_t;


void sample_producer_task(void *pvParameters);
void sample_consumer_task(void *pvParameters);
#endif
