#ifndef _BOARD_ROBO_CFG_H
#define _BOARD_ROBO_CFG_H

class VIM_ASSUME_AS_CPP_FILE;

// ------- Servo LWM: TIM1.{1,2,3} =PA8,PA9,PA10
DEFINE_TIMER_DATA_PWM3( SERVOLWM, 1, TimCh1, TimCh2, TimCh3,  GPIO_AF1_TIM1, PA8, PA9, PA10 );
#define TIM_SERVOLWM addr2TIM( TIM_SERVOLWM_BASE )
inline constexpr uint32_t SERVOLWM_FREQ { 100 };


// ------- ADC: A5, A6, A7 {5,6,7} - only if needed, B0 = ch88
#ifdef _OXC_ADC_H
#define ADC_SENSOR ADC1
inline constexpr AdcChannelInfo ADC_SENSOR_CHPINS[] {
  { ADC_CHANNEL_5, PA5 },
  { ADC_CHANNEL_6, PA6 },
  { ADC_CHANNEL_7, PA7 },
  {             0, PBAD } // END
};
inline const constexpr uint32_t ADC_SENSOR_N_CH = std::size( ADC_SENSOR_CHPINS ) - 1;
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

#endif

