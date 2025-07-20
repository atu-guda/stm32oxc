#ifndef _MOMEAS_H
#define _MOMEAS_H

extern TIM_HandleTypeDef htim_cnt; // TIM2
extern TIM_HandleTypeDef htim_cnt2; // TIM3
extern TIM_HandleTypeDef htim_pwm; // TIM5
int MX_TIM_CNT_Init(void);
int MX_TIM_CNT2_Init(void);
int MX_TIM_PWM_Init(void);


#endif

