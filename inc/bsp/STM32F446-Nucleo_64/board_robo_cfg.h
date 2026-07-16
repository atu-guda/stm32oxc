#ifndef _BOARD_ROBO_CFG_H
#define _BOARD_ROBO_CFG_H

// ------- Motor PWM control: left/right + break/... TIM3.1=PC4 (AF2), PC5, PC6

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


// ------- Servo LWM: TIM13.1=PA6(AF9)

inline constexpr PortPin ServoLwmPwmPin   { PA6 };

#define TIM_SERVOLWM_NUM 13

#define TIM_SERVOLWM_BASE OXC_EVAL3( TIM, TIM_SERVOLWM_NUM, _BASE )
#define TIM_SERVOLWM      OXC_EVAL2( TIM, TIM_SERVOLWM_NUM )
#define TIM_SERVOLWM_CLKEN  OXC_EVAL3( __TIM, TIM_SERVOLWM_NUM, _CLK_ENABLE()  );
#define TIM_SERVOLWM_CLKDIS OXC_EVAL3( __TIM, TIM_SERVOLWM_NUM, _CLK_DISABLE() );

inline constexpr std::array tim_servolwm_chspins {
  TimChPin {  TimCh1, GPIO_AF9_TIM13, ServoLwmPwmPin }
};


// ------- STEP: TIM8.3=PC8(AF3) = step, PC9 = dir

inline constexpr PortPin StepStepPin   { PC8 };
inline constexpr PortPin StepDirPin    { PC9 };

#define TIM_STEP_NUM 8

#define TIM_STEP_BASE OXC_EVAL3( TIM, TIM_STEP_NUM, _BASE )
#define TIM_STEP_EVAL2( TIM, TIM_STEP_NUM )
#define TIM_STEP_CLKEN  OXC_EVAL3( __TIM, TIM_STEP_NUM, _CLK_ENABLE()  );
#define TIM_STEP_CLKDIS OXC_EVAL3( __TIM, TIM_STEP_NUM, _CLK_DISABLE() );

inline constexpr std::array tim_step_chspins {
  TimChPin {  TimCh3, GPIO_AF3_TIM8, StepStepPin }
};


// ------- ENC: TIM1.{1,2}=PA8,PA9(AF1)

inline constexpr PortPin EncoderAPin   { PA8 };
inline constexpr PortPin EncoderBPin   { PA9 };

#define TIM_ENCODER_NUM 1

#define TIM_ENCODER_BASE OXC_EVAL3( TIM, TIM_ENCODER_NUM, _BASE )
#define TIM_ENCODER_EVAL2( TIM, TIM_ENCODER_NUM )
#define TIM_ENCODER_CLKEN  OXC_EVAL3( __TIM, TIM_ENCODER_NUM, _CLK_ENABLE()  );
#define TIM_ENCODER_CLKDIS OXC_EVAL3( __TIM, TIM_ENCODER_NUM, _CLK_DISABLE() );

#define TEM_ENCODER_AF GPIO_AF1_TIM1


// ------- ADC: A0, A1, A4, B0 {0,1,4,8}


#endif

