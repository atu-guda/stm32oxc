#ifndef _OXC_ADC_H
#define _OXC_ADC_H

#include <oxc_base.h>

struct AdcSampleTimeInfo {
  uint32_t code;
  uint32_t stime10; //* in 10 * ADC_Cycles
};

extern const unsigned adc_arch_sampletimes_n;
extern const AdcSampleTimeInfo adc_arch_sampletimes[];

struct ADC_freq_info {
  uint32_t freq_in; //* input frequency
  uint32_t freq;    //* working freq
  uint32_t div;     //* total divider
  uint32_t div1;    //* base divider
  uint32_t div2;    //* second divider
  uint32_t devbits; //* misc bits about device: H7: 1: sync mode, 2: ADC_VER_V5_3, 4: REV_ID_Y
};


struct ADC_Info {
  ADC_HandleTypeDef hadc;
  DMA_HandleTypeDef hdma_adc;
  int32_t n_ch_max;
  uint32_t adc_clk = -1;
  uint16_t *data = nullptr;
  float t_step_f = 0; // in s, recalculated before measurement
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

uint32_t ADC_getFreqIn( ADC_HandleTypeDef* hadc );
uint32_t ADC_calc_div( ADC_HandleTypeDef* hadc, uint32_t freq_max, uint32_t *div_val );
uint32_t ADC_conv_time_tick( uint32_t s_idx, uint32_t n_ch, uint32_t n_bits );
uint32_t ADC_calcfreq( ADC_HandleTypeDef* hadc, ADC_freq_info *fi );

#endif

// vim: path=.,/usr/share/stm32cube/inc
