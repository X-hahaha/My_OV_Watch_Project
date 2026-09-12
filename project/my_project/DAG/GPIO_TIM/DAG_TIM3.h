#ifndef __DAG_TIM3_H__
#define __DAG_TIM3_H__
#include "stm32f4xx_hal.h"
#include "stdint.h"
void PWM_SetDuty(TIM_HandleTypeDef *htim, uint32_t channel, float duty);

#endif

