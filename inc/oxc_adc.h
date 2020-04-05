#ifndef _OXC_ADC_H
#define _OXC_ADC_H

// beware: some functions is arch-dependent, and inplemented in src/arch/${arch}/oxc_arch_adc.cpp
//         other - common, in src/axc_adc.cpp

#include <oxc_base.h>
#include <oxc_gpio.h>

struct AdcChannelInfo {
  uint32_t channel; // like ADC_CHANNEL_17
  GpioRegs &gpio;   // like GpioA
  uint8_t  pin_num; // like 15 - not bit
};

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
  const AdcChannelInfo *ch_info = nullptr;
  int32_t prepared = 0;
  uint32_t n_ch_max = 0;
  uint32_t sampl_cycl_common = 0;
  uint32_t adc_clk = -1;
  uint16_t *data = nullptr;
  float t_step_f = 0; // in s, recalculated before measurement
  volatile int end_dma = 0;
  volatile int dma_error = 0;
  volatile uint32_t n_series = 0;
  // volatile uint32_t n_series_todo;
  volatile uint32_t n_good = 0;
  volatile uint32_t n_bad = 0;

  uint32_t last_SR = 0;
  uint32_t good_SR = 0;
  uint32_t bad_SR = 0;
  uint32_t last_end = 0;
  uint32_t last_error = 0;
  uint32_t last_status = 0;


  [[deprecated]] ADC_Info() { reset_cnt(); } // TODO: remove
  ADC_Info( ADC_TypeDef* _hadc, const AdcChannelInfo *ch_i );
  uint32_t set_channels( const AdcChannelInfo *ch_i );
  void reset_cnt();
  uint32_t init_gpio_channels();
  uint32_t start_DMA_wait( uint32_t n_ch, uint32_t n, uint32_t t_wait ); // 0 - ok, 1 - arg error, 2 - start err 3 - no end DMA, 4 - DMA error
  uint32_t start_DMA_wait_n( uint32_t n_ch, uint32_t n, uint32_t t_wait, uint32_t chank_sz );
  uint32_t init_adc_channels(); // arch-dependent, in oxc_arch_adc.cpp

  uint32_t prepare_single_manual( uint32_t presc, uint32_t sampl_cycl, uint32_t resol ); // arch-dep
  uint32_t prepare_multi_ev( uint32_t n_ch, uint32_t presc, uint32_t sampl_cycl, uint32_t ev, uint32_t resol ); // arch-dep
  uint32_t prepare_multi_ev_n( uint32_t n_ch, uint32_t presc, uint32_t sampl_cycl, uint32_t ev, uint32_t resol ); // arch-dep

  int DMA_reinit( uint32_t mode );
  void convCpltCallback( ADC_HandleTypeDef *hadc );
  void convHalfCpltCallback( ADC_HandleTypeDef *hadc );
  void errorCallback( ADC_HandleTypeDef *hadc );

  uint32_t init_xxx1(); // arch-dep
  uint32_t start() { if( !prepared ) return 0; return HAL_ADC_Start( &hadc ) == HAL_OK; }; // TODO: more check
  bool poll_and_read( int &v );
};

struct AdcDma_n_status {
  uint32_t base_dma_addr;
  uint32_t next_dma_ofs;
  uint32_t step_dma_addr;
  uint32_t dma_total_sz;
};
extern AdcDma_n_status adcdma_n_status;

// TODO: different for F1, F3
// [[deprecated]]
const uint32_t adc_n_sampl_times = 7;
extern const uint32_t sampl_times_codes[adc_n_sampl_times];
extern const uint32_t sampl_times_cycles[adc_n_sampl_times];

extern "C" {
 void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc );
 void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc );
}

[[deprecated]] uint32_t calc_ADC_clk( uint32_t presc, int *div_val );
[[deprecated]] uint32_t hint_ADC_presc();
[[deprecated]] void pr_ADC_state( const ADC_Info &adc );

uint32_t ADC_getFreqIn( ADC_HandleTypeDef* hadc );
uint32_t ADC_calc_div( ADC_HandleTypeDef* hadc, uint32_t freq_max, uint32_t *div_val );
uint32_t ADC_conv_time_tick( uint32_t s_idx, uint32_t n_ch, uint32_t n_bits );
uint32_t ADC_calcfreq( ADC_HandleTypeDef* hadc, ADC_freq_info *fi );

HAL_StatusTypeDef ADC_Start_DMA_n( ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length, uint32_t chunkLength );

#endif

// vim: path=.,/usr/share/stm32cube/inc
