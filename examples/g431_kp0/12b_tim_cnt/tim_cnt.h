/*
 *       Filename:  tim_cnt.h
 *    Description:  local definitions for 12b_tim_cnt example
 */
#ifndef _TIM_CNT_H
#define _TIM_CNT_H

#define TIM_IN TIM2
#define TIM_IN_EN    __HAL_RCC_TIM2_CLK_ENABLE();  __HAL_RCC_GPIOA_CLK_ENABLE();
#define TIM_IN_DIS   __HAL_RCC_TIM2_CLK_DISABLE();
#define TIM_IN_PIN   GPIO_PIN_0
#define TIM_IN_AF    GPIO_AF1_TIM2
#define TIM_IN_GPIO  GpioA


#endif

