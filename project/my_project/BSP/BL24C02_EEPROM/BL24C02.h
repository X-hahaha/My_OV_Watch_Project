#ifndef __BL24C02_H__
#define __BL24C02_H__
#include "iic_hal.h"

#define BL_ADDRESS	0x50



typedef enum
{
    BL24C02_ERROR = 0,
    BL24C02_SUCCESS = 1
}BL24C02_Status_t;

BL24C02_Status_t BL24C02_Write(uint8_t addr,uint8_t length,uint8_t buff[]);
BL24C02_Status_t BL24C02_Read(uint8_t addr, uint8_t length, uint8_t *buff);
void BL24C02_Init(void);


#endif
