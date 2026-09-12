#include "stm32f4xx_hal.h"
#include "stdint.h"
/**
 * @brief  设置 PWM 占空比（百分比 0~100）
 * @param  htim: TIM 句柄
 * @param  channel: TIM_CHANNEL_x
 * @param  duty: 占空比 0~100（浮点）
 */
void PWM_SetDuty(TIM_HandleTypeDef *htim, uint32_t channel, float duty)
{
    if (duty > 100.0f) duty = 100.0f;
    if (duty < 0.0f)   duty = 0.0f;

    // ARR + 1 是一个周期的总计数
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(htim);
    uint32_t pulse = (uint32_t)((duty / 100.0f) * (arr + 1));

    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}




