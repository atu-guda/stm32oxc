#include <oxc_auto.h>
#include <oxc_adcdata.h>
#include <oxc_adcdata_cmds.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure ADC data (4ch) with DMA double-buffer T2 shot" NL
 " var t - delay time in us" NL
 " var n - default number of measurements" NL
 " var s - sample time index" NL
 " var c - number of channels" NL
 " var b - supress normal output" NL
 " var v - reference voltage in uV " NL;

const AdcChannelInfo adc_channels[] = {
  { BOARD_ADC_DEFAULT_CH0, BOARD_ADC_DEFAULT_GPIO0, BOARD_ADC_DEFAULT_PIN0 },
  { BOARD_ADC_DEFAULT_CH1, BOARD_ADC_DEFAULT_GPIO1, BOARD_ADC_DEFAULT_PIN1 },
  { BOARD_ADC_DEFAULT_CH2, BOARD_ADC_DEFAULT_GPIO2, BOARD_ADC_DEFAULT_PIN2 },
  { BOARD_ADC_DEFAULT_CH3, BOARD_ADC_DEFAULT_GPIO3, BOARD_ADC_DEFAULT_PIN3 },
  {                     0,                   GpioA,                    255 } // END
};

const uint32_t ADCDMA_chunk_size = 1024; // in bytes, for now. may be up to 64k-small
// const uint32_t ADCDMA_chunk_size = 8 * 4 * 2; // debug: 8 lines
const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX;
using AdcDataX = AdcData<BOARD_ADC_DEFAULT_BITS,xfloat>;
AdcDataX adcd( BOARD_ADC_MALLOC, BOARD_ADC_FREE );

// tmp: for debug DMA
// uint32_t  *xxx_dma_isr;
char tmp_log_buff[4096];

const int tim_base_freq = 1000000;

TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc, uint32_t arr );
void tim2_deinit();

ADC_Info adc( BOARD_ADC_DEFAULT_DEV, adc_channels );

int v_adc_ref = BOARD_ADC_COEFF; // in uV, measured before test


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] - test ADC"  };
int cmd_set_coeffs( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_COEFFS { "set_coeffs", 'F', cmd_set_coeffs, " k0 k1 k2 k3 - set ADC coeffs"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " [N [start]]- output data "  };
int cmd_outhex( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTHEX { "outhex", 'H', cmd_outhex, " [N [start]]- output data in hex form"  };
int cmd_show_stats( int argc, const char * const * argv );
CmdInfo CMDINFO_SHOWSTATS { "show_stats", 'Y', cmd_show_stats, " [N [start]]- show statistics"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SET_COEFFS,
  &CMDINFO_OUT,
  &CMDINFO_OUTHEX,
  &CMDINFO_SHOWSTATS,
  nullptr
};



int main(void)
{
  BOARD_PROLOG;

  UVAR('d') = 0;      // TMP: debug
  UVAR('t') = 1000; // TMP: 0.01 for debug, real: 1000; // 1 ms
  UVAR('n') = 20;    // must be rounded to 24 in small chunks (64B)
  UVAR('c') = adc.n_ch_max; // number of channels
  UVAR('s') = adc_arch_sampletimes_n - 1;
  UVAR('v') = v_adc_ref;

  set_log_buf( tmp_log_buff, size(tmp_log_buff) );

  // TODO: check
  #ifdef PWR_CR1_ADCDC1
  // PWR->CR1 |= PWR_CR1_ADCDC1;
  #endif

  BOARD_POST_INIT_BLINK;

  std_out << NL "##################### " PROJ_NAME NL;

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}



// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  const uint32_t n_ch = clamp<uint32_t>( UVAR('c'), 1, adc.n_ch_max );
  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t unsigned stime_idx = ( (uint32_t)UVAR('s') < adc_arch_sampletimes_n ) ? UVAR('s') : (adc_arch_sampletimes_n - 1);
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, n_ADC_series_max ); // number of series

  // make n a multiple of ADCDMA_chunk_size
  uint32_t lines_per_chunk = ADCDMA_chunk_size / ( n_ch  * 2 ); // /2 - 16 bit
  n = ( ( n  + lines_per_chunk - 1 ) / lines_per_chunk ) * lines_per_chunk;

  const uint32_t t_step_us = UVAR('t');
  adc.t_step_f = (decltype(adc.t_step_f))(1e-6f) * t_step_us;
  const xfloat freq_sampl = (xfloat)1e6f / t_step_us;

  const uint32_t adc_arch_clock_in = ADC_getFreqIn( &adc.hadc );
  uint32_t s_div = 0;
  uint32_t div_bits = ADC_calc_div( &adc.hadc, ADC_FREQ_MAX, &s_div );

  std_out <<  NL "# Test0: n= " << n << " n_ch= " << n_ch
    << " t= " << t_step_us << " us, freq_sampl= " << freq_sampl
    << " freq_in= " << adc_arch_clock_in
    << " freq_max= " << BOARD_ADC_FREQ_MAX << NL;

  if( s_div == 0  ||  div_bits == 0xFFFFFFFF ) {
    std_out << "# error: fail to calc divisor" NL;
    return 7;
  }
  const uint32_t adc_freq = adc_arch_clock_in / s_div;
  adc.adc_clk = adc_freq; // TODO: place in good? place
  const uint32_t adc_clock_ns = (unsigned)( ( 1000000000LL + adc_freq - 1 ) / adc_freq );
  std_out << "# div= " << s_div << " bits: " << HexInt( div_bits ) << " freq: " << adc_freq
          << " adc_clock_ns: " << adc_clock_ns << NL;

  uint32_t stime_ns = ADC_conv_time_tick( stime_idx, n_ch, BOARD_ADC_DEFAULT_BITS );
  if( stime_ns == 0xFFFFFFFF ) {
    std_out << "# error: fail to calculate conversion time" NL;
    return 8;
  }
  stime_ns *= adc_clock_ns;

  uint32_t t_wait0 = 2 + uint32_t( n * t_step_us / 1000 ); // in ms

  if( t_step_us * 1000 <= stime_ns ) {
    std_out << "# warn: time step (" << t_step_us * 1000 << ") ns < conversion time (" << stime_ns << ") ns" NL;
  }

  std_out << "# stime_idx= " << stime_idx << " 10ticks= " << adc_arch_sampletimes[stime_idx].stime10
          << " stime_ns= "  << stime_ns  << " code= " <<  adc_arch_sampletimes[stime_idx].code
          << " t_wait0= " << t_wait0 << " ms" NL;

  uint32_t psc = calc_TIM_psc_for_cnt_freq( tim2h.Instance, tim_base_freq ); // 1 us each
  UVAR('p') = psc;
  delay_ms( 1 );

  adc.prepare_multi_ev_n( n_ch, div_bits, adc_arch_sampletimes[stime_idx].code, BOARD_ADC_DEFAULT_TRIG, BOARD_ADC_DEFAULT_RESOLUTION );

  if( ! adc.init_common() ) {
    std_out << "# error: fail to init ADC: errno= " << errno << NL;
  }

  if( UVAR('d') > 0 ) {
    adc.pr_state();
  }
  if( UVAR('d') > 1 ) {
    dump32( BOARD_ADC_DEFAULT_DEV, 0x100 );
  }
  // log_reset();

  // or such
  // ADC_freq_info fi;
  // ADC_calcfreq( &adc.hadc, &fi );
  // std_out << "# ADC: freq_in: " << fi.freq_in << " freq: " << fi.freq
  //         << " div: " << fi.div << " div1: " << fi.div1 << " div2: " << fi.div2
  //         << " bits: " << HexInt( fi.devbits ) << NL;


  // really need for H7 - DMA not work with ordinary memory
  adcd.free();
  if( ! adcd.alloc( n_ch, n, ADCDMA_chunk_size ) ) {
    std_out << "# Error: fail to alloc buffer" << NL;
    return 2;
  }
  adc.data = adcd.data();
  adc.reset_cnt();
  adcd.set_d_t( t_step_us * 1e-6f );
  adcd.set_v_ref_uV( UVAR('v') );
  adcd.fill( 0 ); // debug?
  std_out << "# n_col= " << adcd.get_n_col() << " n_row= " << adcd.get_n_row() << " data: " << HexInt(adcd.data()) << " size_all= " << adcd.size_all() << NL;

  leds.reset( BIT0 | BIT1 | BIT2 );
  int rc = 0;
  break_flag = 0;
  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  if( UVAR('l') ) {  leds.set( BIT2 ); }
  tim2_init( psc, t_step_us - 1 );
  uint32_t r = adc.start_DMA_wait_n( n_ch, n, t_wait0, ADCDMA_chunk_size );
  tim2_deinit();
  if( UVAR('l') ) {  leds.reset( BIT2 ); }

  uint32_t tcc = HAL_GetTick();
  delay_ms( 10 ); // to settle all

  if( r != 0 ) {
    std_out <<  "# error: start_DMA_wait " << r << NL;
    rc = 1;
  }

  std_out <<  "#  tick: " <<  ( tcc - tm00 )  << NL;

  if( UVAR('d') > 0 ) {
    adc.pr_state();
  }
  if( UVAR('d') > 1 ) {
    dump32( BOARD_ADC_DEFAULT_DEV, 0x200 );
  }
  HAL_ADC_Stop_DMA( &adc.hadc ); // ????? not?

  adc.data = nullptr;

  return rc;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != BOARD_ADC_DEFAULT_DEV ) {
    return;
  }
  // std_out << "# debug: " << __PRETTY_FUNCTION__ << NL;
  BOARD_ADC_DEFAULT_EN;
  BOARD_ADC_DMA_DEFAULT_EN;

  adc.init_gpio_channels();

  adc.DMA_reinit( BOARD_ADC_DEFAULT_DBLBUF );

  HAL_NVIC_SetPriority( BOARD_ADC_DMA_IRQ, 2, 0 );
  HAL_NVIC_EnableIRQ(   BOARD_ADC_DMA_IRQ );

  // HAL_NVIC_SetPriority( BOARD_ADC_IRQ, 3, 0 );
  // HAL_NVIC_EnableIRQ( BOARD_ADC_IRQ ); // not need even for adcdma_n
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != BOARD_ADC_DEFAULT_DEV ) {
    return;
  }
  // std_out << "# debug: " << __PRETTY_FUNCTION__ << NL;

  BOARD_ADC_DMA_DEFAULT_DIS;
  BOARD_ADC_DEFAULT_DIS;
}

void HAL_ADC_ConvHalfCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT1 );
  // log_add( "C.H " );
  // ++dbg_val0;
  adc.convHalfCpltCallback( hadc );
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT1 );
  // log_add( "C.C " );
  // ++dbg_val1;
  adc.convCpltCallback( hadc );
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT0 );
  // log_add( "C.E " );
  // ++dbg_val2;
  adc.errorCallback( hadc );
}

void BOARD_ADC_DMA_IRQHANDLER(void)
{
  // leds.set( BIT1 );

  // uint32_t tc = HAL_GetTick();

  // DMA_TypeDef *dma_base = (DMA_TypeDef*)(adc.hdma_adc.StreamBaseAddress);
  // DMA_Stream_TypeDef *dma_stream = (DMA_Stream_TypeDef *)(adc.hdma_adc.Instance);
  // *xxx_dma_isr++ = dma_base->LISR;
  // *xxx_dma_isr++ = dma_stream->CR;
  // // *xxx_dma_isr++ = dma_stream->NDTR;
  // *xxx_dma_isr++ = dma_stream->M0AR;
  // *xxx_dma_isr++ = dma_stream->M1AR;
  // // ++dbg_val3;

  // log_add( "I.AD" );
  // log_add_hex( dma_base->LISR );
  // log_add_hex( dma_stream->CR );
  // log_add_hex( tc );
  // log_add( "[ " );

  HAL_DMA_IRQHandler( &adc.hdma_adc );

  // *xxx_dma_isr++ = dma_base->LISR;
  // *xxx_dma_isr++ = dma_stream->CR;
  // // *xxx_dma_isr++ = dma_stream->NDTR;
  // *xxx_dma_isr++ = dma_stream->M0AR;
  // *xxx_dma_isr++ = dma_stream->M1AR;
  // log_add( " ].AD" NL NL );
  // leds.reset( BIT1 );
}


// unused
// void BOARD_ADC_IRQHANDLER(void)
// {
//   HAL_ADC_IRQHandler( &adc.hadc );
//   leds.toggle( BIT0 );
// }

int cmd_set_coeffs( int argc, const char * const * argv )
{
  return subcmd_set_coeffs( argc, argv, adcd );
}

int cmd_out( int argc, const char * const * argv )
{
  return subcmd_out_any( argc, argv, adcd, false );
}

int cmd_outhex( int argc, const char * const * argv )
{
  return subcmd_out_any( argc, argv, adcd, true );
}


int cmd_show_stats( int argc, const char * const * argv )
{
  // auto ns = adcd.get_n_row();
  // uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  // uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  StatIntData sdat( adcd );
  sdat.slurp( adcd ); // TODO: limits
  sdat.calc();
  std_out << sdat;

  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

