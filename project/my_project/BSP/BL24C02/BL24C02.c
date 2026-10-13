#include "BL24C02.h"
#include "main.h"

static IIC_Bus_t BL24C02_IIC_Bus = {PROM_SDA_GPIO_Port, PROM_SCL_GPIO_Port, PROM_SDA_Pin, PROM_SCL_Pin};


BL24C02_Status_t BL24C02_Write(uint8_t addr,uint8_t length,uint8_t *buff)
{
    uint8_t timeout = 0;
    if(IIC_Write_Multi_Bytes(&BL24C02_IIC_Bus, BL_ADDRESS, addr, buff, length) == IIC_SUCCESS)
    {
        do
        {
            if(timeout > 20)
            {
                return BL24C02_ERROR;
            }
            timeout++;
            HAL_Delay(5);
            IIC_Start(&BL24C02_IIC_Bus);
            IIC_SendByte(&BL24C02_IIC_Bus, (BL_ADDRESS << 1));
        } while (IICWaitAck(&BL24C02_IIC_Bus) == IIC_NotACK);  /* 循环发送起始条件加器件地址等待应答，如若应答则代表数据已经成功写进了*/
        
        return BL24C02_SUCCESS;
    }
    else
    {
        return BL24C02_ERROR;
    }
}

BL24C02_Status_t BL24C02_Read(uint8_t addr, uint8_t length, uint8_t *buff)
{
    if(IIC_Read_Multi_Bytes(&BL24C02_IIC_Bus, BL_ADDRESS, addr, buff, length) == IIC_SUCCESS)
    {
        return BL24C02_SUCCESS;
    }
    else
    {
        return BL24C02_ERROR;
    }
}

void BL24C02_Init(void)
{
	IIC_Init(&BL24C02_IIC_Bus);
}
