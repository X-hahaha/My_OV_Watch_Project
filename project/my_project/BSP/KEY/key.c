#include "key.h"


volatile uint8_t UserReady = 0;
EventGroupHandle_t key_event_group_handle = NULL; //按键事件组

static volatile uint8_t key1_processing = 0; //按键1是否正在处理中
static volatile uint8_t key2_processing = 0; //按键2是否正在处理中

/*
 * @brief: 按键实例初始化
 * @param: key: 指向按键实例的指针
 * @param: InstanceConfig: 按键实例配置结构体
 * @return: 无
*/
void KeyInstance_Init(key_instance_t* key, key_InstanceConfig_t InstanceConfig)
{
    key->GPIO_Port = InstanceConfig.GPIO_Port;
    key->GPIO_Pin = InstanceConfig.GPIO_Pin;
    key->GPIO_PinState_Press = InstanceConfig.GPIO_PinState_Press;
    key->debounce_ms = InstanceConfig.debounce_ms;
    key->long_press_ms = InstanceConfig.long_press_ms;
    key->short_press_release_callback = InstanceConfig.short_press_release_callback;
    key->long_press_release_callback = InstanceConfig.long_press_release_callback;
    key->key_id = InstanceConfig.key_id;
}

void Key_SetCallback(key_instance_t* key, 
                     void (*short_press_release_callback)(key_instance_t* key),
                     void (*long_press_release_callback)(key_instance_t* key))
{
    key->short_press_release_callback = short_press_release_callback;
    key->long_press_release_callback = long_press_release_callback;    
} 

/*
 * @brief: 按键中断回调服务函数
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    if(GPIO_Pin == KEY1_Pin && !key1_processing && UserReady)//等待任务处理完毕再响应
    {
        xEventGroupSetBitsFromISR(key_event_group_handle, KEY1_EVENT_EDGE, &pxHigherPriorityTaskWoken);
        
    }
    else if(GPIO_Pin == WAKE_Pin && !key2_processing && UserReady)
    {
        xEventGroupSetBitsFromISR(key_event_group_handle, KEY2_EVENT_EDGE, &pxHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}
/*
 * @brief: 按键状态机
 * @param: key: 指向按键实例的指针
 * @return: 无
*/
void key_process(key_instance_t* key)
{
    uint8_t volatile *key_processing = NULL;
    KeyState_t key_state = key->state;
    uint8_t key_long_press_flag = 0;
    TickType_t last_tick;
    if(key->key_id == KEY1_EVENT_EDGE)      key_processing = &key1_processing;
    else if(key->key_id == KEY2_EVENT_EDGE) key_processing = &key2_processing;
    
    if(key_state != KEY_STATE_IDLE || !KEY_PRESS(key)) 
    {
        *key_processing = 0;
        return;
    }
    *key_processing = 1;
    key_state = KEY_STATE_PRESS_DEBOUNCE;

    while(1)
    {
        switch(key_state)
        {
            case KEY_STATE_PRESS_DEBOUNCE:
                vTaskDelay(pdMS_TO_TICKS(key->debounce_ms));
                if(KEY_PRESS(key))
                {
                    key_state = KEY_STATE_PRESS;
                    last_tick = xTaskGetTickCount();
                }
                else
                {
                    key_state = KEY_STATE_IDLE;
                }
                break;
            case KEY_STATE_PRESS:
                if(!KEY_PRESS(key))
                {
                    key_state = KEY_STATE_RELEASE_DEBOUNCE;
                }
                vTaskDelay(pdMS_TO_TICKS(5));
                if(xTaskGetTickCount() - last_tick >= pdMS_TO_TICKS(key->long_press_ms))
                {
                    key_long_press_flag = 1;
                }
                break;
            case KEY_STATE_RELEASE_DEBOUNCE:
                vTaskDelay(pdMS_TO_TICKS(key->debounce_ms));
                if(!KEY_PRESS(key))
                {
                    key_state = KEY_STATE_RELEASE;
                }
                else
                {
                    key_state = KEY_STATE_PRESS;
                }
                break;
            case KEY_STATE_RELEASE:
                if(key_long_press_flag)
                {
                    if(key->long_press_release_callback != NULL)  key->long_press_release_callback(key);
                }
                else
                {
                    if(key->short_press_release_callback != NULL)  key->short_press_release_callback(key); 
                }
                key_state = KEY_STATE_IDLE;
                break;
            case KEY_STATE_IDLE:
                key->state = KEY_STATE_IDLE;
                *key_processing = 0;
                return;
            default:
                break;
        }
        key->state = key_state;
    }
}