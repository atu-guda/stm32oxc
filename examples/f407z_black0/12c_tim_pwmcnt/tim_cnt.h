/*
 *       Filename:  tim_cnt.h
 *    Description:  local definitions for 12b_tim_cnt example
 */
#ifndef _TIM_CNT_H
#define _TIM_CNT_H

#define TIM_IN TIM5
#define TIM_IN_EN    __HAL_RCC_TIM5_CLK_ENABLE();
#define TIM_IN_DIS   __HAL_RCC_TIM5_CLK_DISABLE();
#define TIM_IN_PIN   PA0
#define TIM_IN_AF    GPIO_AF2_TIM5


#endif

