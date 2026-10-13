#include "stm32f4xx_hal.h"
#include "stdint.h"

extern TIM_HandleTypeDef   htim10;


void delay_ms(uint32_t nms)
{
    HAL_Delay(nms);
}
/*
 * @brief  延时nus微秒  
 * @param  nus: 微秒延时时间
 * @Note   TIM10配置：计时时钟1Mhz，计时周期1ms，即ARR为999
*/
void delay_us(uint32_t nus)
{
    uint32_t ms = nus / 1000;
    uint32_t us_remain = nus % 1000;

    if(ms > 0)
    {
        delay_ms(ms);
    }
    if(us_remain > 0)
    {
        uint16_t tick_start = __HAL_TIM_GET_COUNTER(&htim10);
        uint16_t tick_now;
        uint16_t elapsed_us;
        do
        {
            tick_now = __HAL_TIM_GET_COUNTER(&htim10);
            if(tick_now >= tick_start)
            {
                elapsed_us = tick_now - tick_start;                //未溢出时经过的时间
            }
            else
            {
                elapsed_us = tick_now + (1000U - tick_start);     //溢出时经过的时间
            }
            
        } while (elapsed_us < (uint16_t)us_remain);
        
    }

}









