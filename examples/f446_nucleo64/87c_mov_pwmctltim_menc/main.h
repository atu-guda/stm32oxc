#ifndef _MAIN_H
#define _MAIN_H

using namespace oxc;

inline constexpr PortPin PwmLeftPin  { PC4  };
inline constexpr PortPin PwmRightPin { PC5  };
inline constexpr PortPin PwmPwmPin   { PC6 };

#define TIM_PWM_NUM 3


//#define TIM_PWM_BASE   OXC_EVAL3( TIM, TIM_PWM_NUM, _BASE )
inline constexpr uint32_t TIM_PWM_BASE { OXC_EVAL3( TIM, TIM_PWM_NUM, _BASE ) };
// TODO: remove + add tim_calc* functions
// inline TIM_TypeDef* TIM_PWM  { OXC_EVAL2( TIM, TIM_PWM_NUM ) };
#define TIM_PWM        OXC_EVAL2( TIM, TIM_PWM_NUM )
#define TIM_PWM_CLKEN  OXC_EVAL3( __TIM, TIM_PWM_NUM, _CLK_ENABLE()  );
#define TIM_PWM_CLKDIS OXC_EVAL3( __TIM, TIM_PWM_NUM, _CLK_DISABLE() );

inline constexpr std::array tim_pwm_chspins {
  TimChPin {  TimCh1, GPIO_AF2_TIM3, PwmPwmPin }
};

struct EasingFunInfo {
  EasingFun f;
  float kv;  // max speed koeff, <=1
};

extern const EasingFunInfo part_fun_info[];

#endif

