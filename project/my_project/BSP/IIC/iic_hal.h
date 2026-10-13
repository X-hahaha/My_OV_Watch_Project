#ifndef __IIC_HAL_H__   
#define __IIC_HAL_H__   

#include "main.h"
#include "stdint.h"
#include "delay.h"

typedef struct 
{
    GPIO_TypeDef* IIC_SDA_PORT;
    GPIO_TypeDef* IIC_SCL_PORT;
    uint16_t IIC_SDA_PIN;
    uint16_t IIC_SCL_PIN;
}IIC_Bus_t;
typedef enum
{
    IIC_NotACK = -2,
    IIC_ERROR = -1,
    IIC_SUCCESS = 0
}IIC_state_t;

void IIC_Init(IIC_Bus_t* IIC_Bus);
void IIC_Start(IIC_Bus_t* IIC_Bus);
void IIC_Stop(IIC_Bus_t* IIC_Bus);
int16_t IICWaitAck(IIC_Bus_t* IIC_Bus);
void IIC_SendACK(IIC_Bus_t* IIC_Bus);
void IIC_SendNACK(IIC_Bus_t* IIC_Bus);
void IIC_SendByte(IIC_Bus_t* IIC_Bus, uint8_t byte);
uint8_t IIC_ReceiveByte(IIC_Bus_t* IIC_Bus);

int16_t IIC_Write_One_Byte(IIC_Bus_t* IIC_Bus, uint8_t device_address, uint8_t reg,uint8_t byte);
int16_t IIC_Write_Multi_Bytes(IIC_Bus_t* IIC_Bus, uint8_t device_address, uint8_t reg,uint8_t* bytes, uint8_t byte_num);
int16_t IIC_Read_One_Byte(IIC_Bus_t* IIC_Bus, uint8_t device_address, uint8_t reg);
int16_t IIC_Read_Multi_Bytes(IIC_Bus_t* IIC_Bus, uint8_t device_address, uint8_t reg,uint8_t* buff, uint8_t num);



#endif

