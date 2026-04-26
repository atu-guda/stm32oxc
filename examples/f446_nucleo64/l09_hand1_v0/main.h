#ifndef _MAIN_H
#define _MAIN_H

// level0: base TB6612FNG + AS5600 C4:C5 - dir+mode
inline constexpr PortPin L0_Ctrl_Pin { PC4  };
inline constexpr PortPin L0_Stop_Pin { PC12 };
// PWM: TIM3.1 C6 AF2
#define TIM_PWM       TIM3
#define TIM_PWM_EN    __HAL_RCC_TIM3_CLK_ENABLE();
#define TIM_PWM_DIS   __HAL_RCC_TIM3_CLK_DISABLE();
#define TIM_PWM_PIN   PC6
#define TIM_PWM_AF    GPIO_AF2_TIM3
#define TIM_PWM_CHANNEL TIM_CHANNEL_1
#define PWM_CCR CCR1
inline constexpr uint32_t tim_pwm_psc_freq   { 42'000'000 }; // 42 MHz - init, TODO: adj after test
inline constexpr uint32_t tim_pwm_freq       {     20'000 }; // 20 kHz - init
extern uint32_t tim_pwm_arr;

extern TIM_HandleTypeDef htim_pwm; // TIM3

int MX_TIM_PWM_Init(void);

#include "lab0_common.h"

bool measure_all();
bool measure_speed( float v );


#endif

