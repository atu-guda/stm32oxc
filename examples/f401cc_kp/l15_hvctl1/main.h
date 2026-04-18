#ifndef _MAIN_H
#define _MAIN_H


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

int measure_adc( int nx );
int measure_press();
bool default_loop( bool can_stop = false );
void default_out( int i );

inline const std::size_t buf_sz_i2c { 32 }; // 20 char + reserve
inline const std::size_t buf_n_i2c  {  4 }; // 4 lines
extern char buf_i2c[buf_n_i2c][buf_sz_i2c];
inline auto& buf_i2c_0 { buf_i2c[0] };
inline auto& buf_i2c_1 { buf_i2c[1] };
inline auto& buf_i2c_2 { buf_i2c[2] };
inline auto& buf_i2c_3 { buf_i2c[3] };

extern uint32_t f_in;
extern uint32_t pressure;
extern uint32_t t_00;
extern uint32_t t_c;
extern uint32_t t_step;

#endif

