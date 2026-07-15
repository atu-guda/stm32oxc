#ifndef _BOARD_ROBO_CFG_H
#define _BOARD_ROBO_CFG_H

// Motor PWM control: left/right + break/...

inline constexpr PortPin MPwmLeftPin  { PC4  };
inline constexpr PortPin MPwmRightPin { PC5  };
inline constexpr PortPin MPwmPwmPin   { PC6 };

#define TIM_MPWM_NUM 3

#define TIM_MPWM_BASE OXC_EVAL3( TIM, TIM_MPWM_NUM, _BASE )
// TODO: remove + add tim_calc* functions
#define TIM_MPWM      OXC_EVAL2( TIM, TIM_MPWM_NUM )
#define TIM_MPWM_CLKEN  OXC_EVAL3( __TIM, TIM_MPWM_NUM, _CLK_ENABLE()  );
#define TIM_MPWM_CLKDIS OXC_EVAL3( __TIM, TIM_MPWM_NUM, _CLK_DISABLE() );

inline constexpr std::array tim_pwm_chspins {
  TimChPin {  TimCh1, GPIO_AF2_TIM3, MPwmPwmPin }
};

// Servo LWM
inline constexpr PortPin ServoLwmPwmPin   { PA6 };

#define TIM_SERVOLWM_NUM 13

#define TIM_SERVOLWM_BASE OXC_EVAL3( TIM, TIM_SERVOLWM_NUM, _BASE )
// TODO: remove + add tim_calc* functions
#define TIM_SERVOLWM      OXC_EVAL2( TIM, TIM_SERVOLWM_NUM )
#define TIM_SERVOLWM_CLKEN  OXC_EVAL3( __TIM, TIM_SERVOLWM_NUM, _CLK_ENABLE()  );
#define TIM_SERVOLWM_CLKDIS OXC_EVAL3( __TIM, TIM_SERVOLWM_NUM, _CLK_DISABLE() );

inline constexpr std::array tim_servolwm_chspins {
  TimChPin {  TimCh1, GPIO_AF9_TIM13, ServoLwmPwmPin }
};


#endif

