#ifndef _BOARD_ROBO_CFG_H
#define _BOARD_ROBO_CFG_H

// ------- Motor PWM control: left/right + break/... TIM3.1=PC6 (AF2), PC4, PC5
DEFINE_TIMER_DATA_PWM1_2P( MPWM, 3, TimCh1, GPIO_AF2_TIM3, PC6, PC4, PC5 );
#define TIM_MPWM addr2TIM( TIM_MPWM_BASE )


// ------- Servo LWM: TIM13.1=PA6(AF9)
DEFINE_TIMER_DATA_PWM1( SERVOLWM, 13, TimCh1, GPIO_AF9_TIM13, PA6 );
#define TIM_SERVOLWM addr2TIM( TIM_SERVOLWM_BASE )
inline constexpr uint32_t SERVOLWM_FREQ { 100 };


// ------- STEP: TIM8.3=PC8(AF3) = step, PC9 = dir
DEFINE_TIMER_DATA_PWM1_1P( STEP, 8, TimCh3, GPIO_AF3_TIM8, PC8, PC9 );
#define TIM_STEP addr2TIM( TIM_STEP_BASE )


// ------- ENC: TIM1.{1,2}=PA8,PA9(AF1)
DEFINE_TIMER_DATA_ENCO( ENCODER, 1, GPIO_AF1_TIM1, PA8, PA9 );
#define TIM_ENCO addr2TIM( TIM_ENCODER_BASE )



// ------- ADC: A0, A1, A4, B0 {0,1,4,8}
#define ADC_SENSOR ADC1
inline constexpr AdcChannelInfo ADC_SENSOR_CHPINS[] {
  { ADC_CHANNEL_0, PA0 },
  { ADC_CHANNEL_1, PA1 },
  { ADC_CHANNEL_4, PA4 },
  { ADC_CHANNEL_8, PB0 },
  {             0, PBAD } // END
};
#define ADC_SENSOR_CLK_EN       __HAL_RCC_ADC1_CLK_ENABLE
#define ADC_SENSOR_CLK_DIS      __HAL_RCC_ADC1_CLK_DISABLE
#define ADC_SENSOR_DMA_CLK_EN   __HAL_RCC_DMA2_CLK_ENABLE
#define ADC_SENSOR_DMA_CLK_DIS  __HAL_RCC_DMA2_CLK_DISABLE
extern ADC_HandleTypeDef hadc_sensor;
extern DMA_HandleTypeDef hdma_adc_sensor;
inline constexpr uint32_t  ADC_SENSOR_SAMPLETIME      { ADC_SAMPLETIME_144CYCLES };
inline constexpr uint32_t  ADC_SENSOR_ClockPrescaler  { ADC_CLOCK_SYNC_PCLK_DIV4 };
inline constexpr uint32_t  ADC_SENSOR_Resolution      { ADC_RESOLUTION_12B };
inline constexpr uint32_t  ADC_SENSOR_DMA_Instance_B  { DMA2_Stream0_BASE };
inline constexpr uint32_t  ADC_SENSOR_DMA_Channel     { DMA_CHANNEL_0 };
inline constexpr IRQn_Type ADC_SENSOR_DMA_IRQ         { DMA2_Stream0_IRQn };
inline constexpr uint32_t  ADC_SENSOR_DMA_IRQ_PRTY    { 4 };


#endif

