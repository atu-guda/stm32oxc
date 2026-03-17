#ifndef _MAIN_H
#define _MAIN_H

int measure_adc( int nx );

inline constexpr std::size_t adc_n_ch { 2 };
extern int32_t  adc_data[adc_n_ch];  // collected and divided data (by adc_measure)
extern uint16_t adc_buf[adc_n_ch];   // buffer for DMA
inline constexpr PortPin ADC1_PIN0 { PA4 };
inline constexpr PortPin ADC1_PIN1 { PA5 };
#define ADC_CLK_EN __HAL_RCC_ADC1_CLK_ENABLE();
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
void MX_DMA_Init(void);
int  MX_ADC1_Init(void);
void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle );
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle);


#endif

