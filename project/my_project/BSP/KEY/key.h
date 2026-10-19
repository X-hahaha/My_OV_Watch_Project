#ifndef __KEY_H__
#define __KEY_H__

#include "main.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define KEY1_EVENT_EDGE            (1 << 0)
#define KEY2_EVENT_EDGE            (1 << 1)
#define Get_Key_PinState(key)      (HAL_GPIO_ReadPin(key->GPIO_Port, key->GPIO_Pin))
#define KEY_PRESS(key)             (Get_Key_PinState(key) == key->GPIO_PinState_Press)

typedef enum
{
    KEY_STATE_IDLE = 0,
    KEY_STATE_PRESS_DEBOUNCE = 1,
    KEY_STATE_PRESS = 2,
    KEY_STATE_LONG_PRESS = 3,
    KEY_STATE_RELEASE_DEBOUNCE = 4,
    KEY_STATE_RELEASE = 5 
}KeyState_t;

typedef struct key_instance
{
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
    GPIO_PinState GPIO_PinState_Press;

    uint16_t debounce_ms;
    uint16_t long_press_ms;

    KeyState_t state;

    void (*short_press_release_callback)(struct key_instance* key);
    void (*long_press_release_callback)(struct key_instance* key);

    uint32_t key_id;

}key_instance_t;

typedef struct 
{
    GPIO_TypeDef* GPIO_Port;
    uint16_t GPIO_Pin;
    GPIO_PinState GPIO_PinState_Press;

    uint16_t debounce_ms;
    uint16_t long_press_ms;

    void (*short_press_release_callback)(struct key_instance* key);
    void (*long_press_release_callback)(struct key_instance* key);  

    uint32_t key_id;
}key_InstanceConfig_t;


typedef enum
{
    KEY_Short_Press = 0,
    KEY_Long_Press = 1,
}KeyPressType_t;

extern volatile uint8_t UserReady;
extern EventGroupHandle_t key_event_group_handle;
void KeyInstance_Init(key_instance_t* key, key_InstanceConfig_t InstanceConfig);
void Key_SetCallback(key_instance_t* key, 
                     void (*short_press_realese_callback)(key_instance_t* key),
                     void (*long_press_realese_callback)(key_instance_t* key));
void key_process(key_instance_t* key);



#endif
