#include "FreeRTOS.h"
#include "task.h"
#include "stdint.h"
#include "stdio.h"
#include "main.h"
#include "usart.h"
#include "Diag.h"
#include "DataSave.h"
#include "semphr.h"
#include "string.h"

TaskHandle_t uart_task_handle = NULL;
TaskHandle_t EEPROM_Save_Read_test_task_handle = NULL;


void UART_test(void *pvParameters)
{
    while(1)
    {
        LOG("boot ok!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void EEPROM_Save_Read_test(void *pvParameters)
{
    uint8_t savestate;
    uint8_t readstate;
    uint8_t EEPROM_DATA[5] = {0x00, 0x01, 0x01, 0x10, 0xAA};
    EEPROM_Init();
    if(EEPROM_Check() == EEPROM_CHECK_OK)
    {
        LOG("EEPROM check success!");
    }
    else
    {
        LOG("EEPROM check failed!");
        FAIL(DIAG_FAIL_EEPROMROM_Check);
    }
    savestate = (uint8_t)SettingSave(&EEPROM_DATA[0], RaiseWristSwitch_addr, 1);
    savestate = (uint8_t)SettingSave(&EEPROM_DATA[1], APPSystemSwitch_addr, 1);
    savestate = (uint8_t)SettingSave(&EEPROM_DATA[2], data_addr, 1);
    savestate = (uint8_t)SettingSave(&EEPROM_DATA[3], step_high_addr, 1);
    savestate = (uint8_t)SettingSave(&EEPROM_DATA[4], step_low_addr, 1);
    if(savestate == EEPROM_SAVE_SUCCESS)
    {
        LOG("EEPROM save success!");
        LOG("Switch:0/1,data:monday,step:4266!");
    }
    else
    {
        LOG("EEPROM save failed!");
    }
    memset(EEPROM_DATA, 0, sizeof(EEPROM_DATA));
    readstate = (uint8_t)SettingGet(&EEPROM_DATA[0], RaiseWristSwitch_addr, 1);
    readstate = (uint8_t)SettingGet(&EEPROM_DATA[1], APPSystemSwitch_addr, 1);
    readstate = (uint8_t)SettingGet(&EEPROM_DATA[2], data_addr, 1);
    readstate = (uint8_t)SettingGet(&EEPROM_DATA[3], step_high_addr, 1);
    readstate = (uint8_t)SettingGet(&EEPROM_DATA[4], step_low_addr, 1);
    if(readstate == EEPROM_GET_SUCCESS)
    {
        LOG("EEPROM read success!");
        LOG("read::Switch:%d/%d, data:%d, step:%d:%d!", 
            EEPROM_DATA[0], EEPROM_DATA[1], EEPROM_DATA[2], EEPROM_DATA[3], EEPROM_DATA[4]);
    }
    else
    {
        LOG("EEPROM read failed!");
    }
    while(1)
    {
        if(savestate == EEPROM_SAVE_SUCCESS && readstate == EEPROM_GET_SUCCESS)
        {
            LOG("EEPROM save & readsuccess!");
            LOG("read::Switch:%d/%d, data:%d, step:%d:%d!", 
                EEPROM_DATA[0], EEPROM_DATA[1], EEPROM_DATA[2], EEPROM_DATA[3], EEPROM_DATA[4]);
        }
        else
        {
            LOG("EEPROM save & read failed!");   
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void APP_Start(void)
{
    /* 开启蓝牙串口模块供电 */
    HAL_GPIO_WritePin(BLE_EN_GPIO_Port, BLE_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1000);

    if(xTaskCreate(UART_test, "UART_test_task", 512, NULL, 2, &uart_task_handle) != pdPASS)
    {
        FAIL(DIAG_FAIL_TaskCreate);
    }
    if((xTaskCreate(EEPROM_Save_Read_test, "EEPROM_Save_Read_test_task", 512, NULL, 2, &EEPROM_Save_Read_test_task_handle)) != pdPASS)
    {
        FAIL(DIAG_FAIL_TaskCreate);
    }
    /* 开启任务调度器 */    
    vTaskStartScheduler();
    FAIL(DIAG_FAIL_SchedulerReturn);    
}




int __io_putchar(int ch)
{
    
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

