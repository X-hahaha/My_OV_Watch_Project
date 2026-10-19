#ifndef __KEY_TASK_H__  
#define __KEY_TASK_H__  

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "key.h"

typedef enum{
    KEY1_Short_Press = 0,
    KEY1_Long_Press = 1,
    KEY2_Short_Press = 2,
    KEY2_Long_Press = 3,
}KeyEvent_t;

extern QueueHandle_t key_queue_handle;
extern key_instance_t Key1_instance;
extern key_instance_t Key2_instance;

void keyscan_task(void *parameters);





#endif


