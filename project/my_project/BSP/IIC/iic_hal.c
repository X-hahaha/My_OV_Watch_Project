
#include "iic_hal.h"


/*整体逻辑：拉低SCL变换SDA，拉高SCL，监听SDA*/

static void SDA_Output(IIC_Bus_t* IIC_Bus, uint8_t val)
{
    if(val)
    {
        IIC_Bus->IIC_SDA_PORT->BSRR = IIC_Bus->IIC_SDA_PIN;
    }
    else
    {
        IIC_Bus->IIC_SDA_PORT->BSRR = (uint32_t)(IIC_Bus->IIC_SDA_PIN << 16);
    }
}

static void SCL_Output(IIC_Bus_t* IIC_Bus, uint8_t val)
{
    if(val)
    {
        IIC_Bus->IIC_SCL_PORT->BSRR = IIC_Bus->IIC_SCL_PIN;
    }
    else
    {
        IIC_Bus->IIC_SCL_PORT->BSRR = (uint32_t)(IIC_Bus->IIC_SCL_PIN << 16);
    }
}

static void SDA_Input_Mode(IIC_Bus_t* IIC_Bus)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = IIC_Bus->IIC_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(IIC_Bus->IIC_SDA_PORT, &GPIO_InitStruct);
}

static void SDA_Output_Mode(IIC_Bus_t* IIC_Bus)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = IIC_Bus->IIC_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;   
    HAL_GPIO_Init(IIC_Bus->IIC_SDA_PORT, &GPIO_InitStruct);
}

static uint8_t SDA_Input(IIC_Bus_t* IIC_Bus)
{
    return (uint8_t)HAL_GPIO_ReadPin(IIC_Bus->IIC_SDA_PORT, IIC_Bus->IIC_SDA_PIN);
}


void IIC_Start(IIC_Bus_t* IIC_Bus)
{
    SDA_Output(IIC_Bus, 1);
    delay_us(2);
    SCL_Output(IIC_Bus, 1);
    delay_us(1);
    SDA_Output(IIC_Bus, 0);
    delay_us(1);
    SCL_Output(IIC_Bus, 0);
    delay_us(1);
}

void IIC_Stop(IIC_Bus_t* IIC_Bus)
{
    SCL_Output(IIC_Bus, 0);
    delay_us(2);
    SDA_Output(IIC_Bus, 0);
    delay_us(1);
    SCL_Output(IIC_Bus, 1);
    delay_us(1);    
    SDA_Output(IIC_Bus, 1);
    delay_us(1);
}

int16_t IICWaitAck(IIC_Bus_t* IIC_Bus)
{
    uint8_t timeout = 5;
    SCL_Output(IIC_Bus, 0);
    delay_us(1);
    SDA_Input_Mode(IIC_Bus);
    SCL_Output(IIC_Bus, 1);
    while(SDA_Input(IIC_Bus))
    {
        timeout--;
        delay_us(1);
        if(timeout == 0)
        {
            SDA_Output_Mode(IIC_Bus);
            IIC_Stop(IIC_Bus);
            delay_us(1);
            return IIC_NotACK;
        }
    }
    SDA_Output_Mode(IIC_Bus);
    SCL_Output(IIC_Bus, 0);
    delay_us(2);
    return IIC_SUCCESS;
}
void IIC_SendACK(IIC_Bus_t* IIC_Bus)
{
    SCL_Output(IIC_Bus, 0);
    delay_us(1);
    SDA_Output(IIC_Bus, 0);
    delay_us(1);
    SCL_Output(IIC_Bus, 1);
    delay_us(1);
    SCL_Output(IIC_Bus, 0);
}
void IIC_SendNACK(IIC_Bus_t* IIC_Bus)
{
    SCL_Output(IIC_Bus, 0);
    delay_us(1);
    SDA_Output(IIC_Bus, 1);
    delay_us(1);
    SCL_Output(IIC_Bus, 1);
    delay_us(1);
    SCL_Output(IIC_Bus, 0);
    delay_us(1);    
}

void IIC_SendByte(IIC_Bus_t* IIC_Bus, uint8_t byte)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        SCL_Output(IIC_Bus, 0);
        delay_us(1);
        SDA_Output(IIC_Bus, (byte << i) & (0x80));
        delay_us(1);
        SCL_Output(IIC_Bus, 1);
        delay_us(1);
    }
    SCL_Output(IIC_Bus, 0);
    delay_us(1);    
}

uint8_t IIC_ReceiveByte(IIC_Bus_t* IIC_Bus)
{
    uint8_t rec_byte = 0;

    SDA_Input_Mode(IIC_Bus);
    for(uint8_t i = 0; i < 8; i++)
    {
        SCL_Output(IIC_Bus, 0);
        delay_us(1);
        SCL_Output(IIC_Bus, 1);
        delay_us(1);
        rec_byte |= (SDA_Input(IIC_Bus) << (7 - i));
    }
    SCL_Output(IIC_Bus, 0);
    delay_us(1);
    SDA_Output_Mode(IIC_Bus);
    return rec_byte;
}

int16_t IIC_Write_One_Byte(IIC_Bus_t* IIC_Bus, uint8_t device_address, uint8_t reg,uint8_t byte)
{
    IIC_Start(IIC_Bus);
    IIC_SendByte(IIC_Bus, (device_address << 1));
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }
    IIC_SendByte(IIC_Bus, reg);
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }
    IIC_SendByte(IIC_Bus, byte);
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }
    IIC_Stop(IIC_Bus);
    return IIC_SUCCESS;
}

int16_t IIC_Write_Multi_Bytes(IIC_Bus_t* IIC_Bus, uint8_t device_address, uint8_t reg,uint8_t* bytes, uint8_t byte_num)
{
    IIC_Start(IIC_Bus);
    IIC_SendByte(IIC_Bus, (device_address << 1));
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }
    IIC_SendByte(IIC_Bus, reg);
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }
    for(uint8_t i = 0; i < byte_num; i++)
    {
        IIC_SendByte(IIC_Bus, bytes[i]);    
        if(IICWaitAck(IIC_Bus) == IIC_NotACK)
        {
            IIC_Stop(IIC_Bus);
            return IIC_NotACK;
        }
    }
    IIC_Stop(IIC_Bus);
    return IIC_SUCCESS;
}
int16_t IIC_Read_One_Byte(IIC_Bus_t* IIC_Bus, uint8_t device_address, uint8_t reg)
{
    uint8_t rec_byte = 0;
    IIC_Start(IIC_Bus);
    IIC_SendByte(IIC_Bus, (device_address << 1));
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }
    IIC_SendByte(IIC_Bus, reg);
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }

    IIC_Start(IIC_Bus);
    IIC_SendByte(IIC_Bus, (device_address << 1) | 0x01);
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }
    rec_byte = IIC_ReceiveByte(IIC_Bus);
    IIC_SendNACK(IIC_Bus);
    IIC_Stop(IIC_Bus);
    return rec_byte;
}

int16_t IIC_Read_Multi_Bytes(IIC_Bus_t* IIC_Bus, uint8_t device_address, uint8_t reg,uint8_t* buff, uint8_t num)
{
    IIC_Start(IIC_Bus);
    IIC_SendByte(IIC_Bus, (device_address << 1));
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }
    IIC_SendByte(IIC_Bus, reg);
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }

    IIC_Start(IIC_Bus);
    IIC_SendByte(IIC_Bus, (device_address << 1) | 0x01);
    if(IICWaitAck(IIC_Bus) == IIC_NotACK)
    {
        IIC_Stop(IIC_Bus);
        return IIC_NotACK;
    }
    for(uint8_t i = 0; i < num; i++)
    {
        buff[i] = IIC_ReceiveByte(IIC_Bus);
        if(i < num - 1)
        {
            IIC_SendACK(IIC_Bus);
        }

    }
    IIC_SendNACK(IIC_Bus);
    IIC_Stop(IIC_Bus);
    return IIC_SUCCESS;
}

/**
 * @brief 初始化IIC总线，配置SDA和SCL引脚为开漏输出，上拉输入
 * 
 * @param IIC_Bus IIC总线结构体指针
 */
void IIC_Init(IIC_Bus_t* IIC_Bus)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = IIC_Bus->IIC_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(IIC_Bus->IIC_SDA_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = IIC_Bus->IIC_SCL_PIN;
    HAL_GPIO_Init(IIC_Bus->IIC_SCL_PORT, &GPIO_InitStruct);
}

