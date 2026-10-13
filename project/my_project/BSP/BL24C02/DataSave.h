#ifndef __DATASAVE_H__
#define __DATASAVE_H__
#include "BL24C02.h"

#define check_word1_addr                   0x00
#define check_word2_addr                   0x01
#define RaiseWristSwitch_addr              0x10
#define APPSystemSwitch_addr               0x11
#define data_addr                          0x20
#define step_high_addr                     0x21
#define step_low_addr                      0x22   
#define check_word1                        0x55
#define check_word2                        0xAA



typedef enum
{
    EEPROM_CHECK_OK = 0,
    EEPROM_CHECK_ERROR = 1,
    EEPROM_SAVE_SUCCESS,
    EEPROM_SAVE_FAIL,
    EEPROM_GET_SUCCESS,
    EEPROM_GET_FAIL
}EEPROM_Check_Status_t;

void EEPROM_Init(void);
EEPROM_Check_Status_t EEPROM_Check(void);
EEPROM_Check_Status_t SettingSave(uint8_t *buf, uint8_t addr, uint8_t lenth);
EEPROM_Check_Status_t SettingGet(uint8_t *buf, uint8_t addr, uint8_t lenth);

#endif
