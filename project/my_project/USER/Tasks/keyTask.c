#include "main.h"
#include "stdint.h"

#include "keyTask.h"

#define KEY1                    KEY1_EVENT_EDGE
#define KEY2                    KEY2_EVENT_EDGE



QueueHandle_t key_queue_handle = NULL; //按键队列  

key_instance_t Key1_instance;
key_instance_t Key2_instance;


static void key1_short_press_realese_callback(key_instance_t* key)
{
    KeyEvent_t key_event = KEY1_Short_Press;
    if(key->key_id != KEY1) return;
    xQueueSend(key_queue_handle, &key_event, 0);
}
static void key1_long_press_realese_callback(key_instance_t* key)
{
    KeyEvent_t key_event = KEY1_Long_Press;
    if(key->key_id != KEY1) return;
    xQueueSend(key_queue_handle, &key_event, 0);
}
static void key2_short_press_realese_callback(key_instance_t* key)
{
    KeyEvent_t key_event = KEY2_Short_Press;
    if(key->key_id != KEY2) return; 
    xQueueSend(key_queue_handle, &key_event, 0);
}
static void key2_long_press_realese_callback(key_instance_t* key)
{
    KeyEvent_t key_event = KEY2_Long_Press;
    if(key->key_id != KEY2) return; 
    xQueueSend(key_queue_handle, &key_event, 0);
}

static void KEY_Instance_Init(void)
{
    key_InstanceConfig_t InstanceConfig;
    InstanceConfig.GPIO_Port = KEY1_GPIO_Port;
    InstanceConfig.GPIO_Pin = KEY1_Pin;
    InstanceConfig.GPIO_PinState_Press = GPIO_PIN_RESET;
    InstanceConfig.debounce_ms = 20;
    InstanceConfig.long_press_ms = 1000;
    InstanceConfig.short_press_release_callback = key1_short_press_realese_callback;
    InstanceConfig.long_press_release_callback = key1_long_press_realese_callback;
    InstanceConfig.key_id = KEY1;
    KeyInstance_Init(&Key1_instance, InstanceConfig);

    InstanceConfig.GPIO_Port = WAKE_GPIO_Port;
    InstanceConfig.GPIO_Pin = WAKE_Pin;
    InstanceConfig.GPIO_PinState_Press = GPIO_PIN_SET;
    InstanceConfig.debounce_ms = 20;
    InstanceConfig.long_press_ms = 1000;
    InstanceConfig.short_press_release_callback = key2_short_press_realese_callback;
    InstanceConfig.long_press_release_callback = key2_long_press_realese_callback;
    InstanceConfig.key_id = KEY2;
    KeyInstance_Init(&Key2_instance, InstanceConfig);
}


/*
 * @brief: 按键扫描任务函数
 * @param: parameters: 指向按键实例的指针
 * @return: 无
*/
void keyscan_task(void *parameters)
{
    KEY_Instance_Init();
    key_instance_t* key = (key_instance_t*)parameters;
    while(1)
    {
        xEventGroupWaitBits(key_event_group_handle, key->key_id,
                            pdTRUE, pdTRUE, portMAX_DELAY);
        key_process(key);
    }
}








