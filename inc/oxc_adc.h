#ifndef _OXC_ADC_H
#define _OXC_ADC_H

#include <oxc_base.h>

struct ADC_Info {
  ADC_HandleTypeDef hadc;
  DMA_HandleTypeDef hdma_adc;
  int32_t n_ch_max;
  uint32_t adc_clk = -1;
  volatile int end_dma;
  volatile int dma_error;
  volatile uint32_t n_series;
  // volatile uint32_t n_series_todo;
  volatile uint32_t n_good;
  volatile uint32_t n_bad;

  uint32_t last_SR;
  uint32_t good_SR;
  uint32_t bad_SR;
  uint32_t last_end;
  uint32_t last_error;


  ADC_Info() { reset_cnt(); }
  void reset_cnt();
};


// TODO: different for F1, F3
const uint32_t adc_n_sampl_times = 7;
extern const uint32_t sampl_times_codes[adc_n_sampl_times];
extern const uint32_t sampl_times_cycles[adc_n_sampl_times];

extern "C" {
 void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc );
 void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc );
}

uint32_t calc_ADC_clk( uint32_t presc, int *div_val );
uint32_t hint_ADC_presc();
void pr_ADC_state( const ADC_Info &adc );


#endif

// vim: path=.,/usr/share/stm32cube/inc
