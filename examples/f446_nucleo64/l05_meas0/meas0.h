#ifndef _MEAS0_H
#define _MEAS0_H

struct D_in_sources {
  decltype( GPIOA ) gpio;
  uint16_t           bit;
};

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim8;

inline const int n_adc_ch = 4;
inline const int n_dac_ch = 2;
inline const int n_din_ch = 4;
inline const int n_pwm_ch = 4;
extern float vref_in;
extern float vref_out;

extern D_in_sources d_ins[n_din_ch];

int MX_BTN_Init();

int MX_TIM1_Init();
int MX_TIM2_Init();
int MX_TIM3_Init();
int MX_TIM4_Init();
int MX_TIM5_Init();
int MX_TIM8_Init();

extern float pwm_out[n_pwm_ch];
void pwm_output();

extern DAC_HandleTypeDef hdac;
int  MX_DAC_Init(void);
void dac_output();
extern float dac_out[n_dac_ch];

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
int  MX_ADC1_Init();
int  MX_DMA_Init(); // used for ADC
int  dma_subinit();

#endif

