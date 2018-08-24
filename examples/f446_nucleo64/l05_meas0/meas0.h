#ifndef _MEAS0_H
#define _MEAS0_H

struct D_in_sources {
  decltype( GPIOA ) gpio;
  uint16_t           bit;
};

inline const int n_adc_ch = 4;
inline const int n_din_ch = 4;
extern float vref;

extern D_in_sources d_ins[n_din_ch];

extern DAC_HandleTypeDef hdac;
int MX_DAC_Init(void);

extern ADC_HandleTypeDef hadc1;
int MX_ADC1_Init();

#endif

