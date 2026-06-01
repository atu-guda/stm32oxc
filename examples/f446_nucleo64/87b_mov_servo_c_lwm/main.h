#ifndef _MAIN_H
#define _MAIN_H

using namespace oxc;

inline constexpr PortPin PwmPwmPin   { PC6 };

#define TIM_PWM_NUM 3

#define TIM_PWM_BASE OXC_EVAL3( TIM, TIM_PWM_NUM, _BASE )
// TODO: remove + add tim_calc* functions
#define TIM_PWM      OXC_EVAL2( TIM, TIM_PWM_NUM )
#define TIM_PWM_CLKEN  OXC_EVAL3( __TIM, TIM_PWM_NUM, _CLK_ENABLE()  );
#define TIM_PWM_CLKDIS OXC_EVAL3( __TIM, TIM_PWM_NUM, _CLK_DISABLE() );

inline constexpr std::array tim_pwm_chspins {
  TimChPin {  TimCh1, GPIO_AF2_TIM3, PwmPwmPin }
};

#endif

