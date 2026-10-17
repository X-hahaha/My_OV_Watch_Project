#include "DataSave.h"
#include "string.h"
void EEPROM_Init(void)
{
    BL24C02_Init();
}

EEPROM_Check_Status_t EEPROM_Check(void)
{
    uint8_t check_buff[2];
    delay_ms(10);
    if(BL24C02_Read(check_word1_addr, 2, check_buff) == BL24C02_SUCCESS)
    {
        if(check_buff[0] == check_word1 && check_buff[1] == check_word2)
        {
            return EEPROM_CHECK_OK;
        }
        else
        {
            check_buff[0] = 0x55;
            check_buff[1] = 0xAA;
            if(BL24C02_Write(0, 2, check_buff) == BL24C02_SUCCESS)
            {
                memset(check_buff, 0, 2);
                BL24C02_Read(0, 2, check_buff);
                if(check_buff[0] == 0x55 && check_buff[1] == 0xAA)
                {
                    return EEPROM_CHECK_OK;
                }
                else
                {
                    return EEPROM_CHECK_ERROR;
                }
            }
            else
            {
                return EEPROM_CHECK_ERROR;
            }
        }
    }
    else
    {
        return EEPROM_CHECK_ERROR;
    }
}


EEPROM_Check_Status_t SettingSave(uint8_t *buf, uint8_t addr, uint8_t lenth)
{
    if(addr > 1 && EEPROM_Check() == EEPROM_CHECK_OK)
    {
        delay_ms(10);
        if(BL24C02_Write(addr, lenth, buf) == BL24C02_SUCCESS)
        {
            return EEPROM_SAVE_SUCCESS;
        }
        else
        {
            return EEPROM_SAVE_FAIL;
        }
    }
    else
    {
        return EEPROM_CHECK_ERROR;
    }
}

EEPROM_Check_Status_t SettingGet(uint8_t *buf, uint8_t addr, uint8_t lenth)
{
    if(addr > 1 && EEPROM_Check() == EEPROM_CHECK_OK)
    {
        delay_ms(10);
        if(BL24C02_Read(addr, lenth, buf) == BL24C02_SUCCESS)
        {
            return EEPROM_GET_SUCCESS;
        }
        else
        {
            return EEPROM_GET_FAIL;
        }
    }
    else
    {
        return EEPROM_CHECK_ERROR;
    }
}


