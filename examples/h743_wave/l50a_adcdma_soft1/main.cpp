#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_adcdata.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test ADC on H7 in manual shot N channels" NL
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

ADC_Info adc( BOARD_ADC_DEFAULT_DEV, adc_channels );
// uint16_t ADC_buf[32];
int ADC_DMA_reinit( ADC_Info &adc ); // TODO: move/remove

int v_adc_ref = BOARD_ADC_COEFF; // in uV, measured before test


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] - test ADC "  };
int cmd_set_coeffs( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_COEFFS { "set_coeffs", 'F', cmd_set_coeffs, " k0 k1 k2 k3 - set ADC coeffs"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SET_COEFFS,
  nullptr
};


const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
float v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100000;
  UVAR('n') = 20;
  UVAR('c') = adc.n_ch_max; // number of channels - 1 for this program, but need for next test
  UVAR('s') = adc_arch_sampletimes_n - 1;
  UVAR('v') = v_adc_ref;

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
  const int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t n_ch = UVAR('c');
  const uint32_t t_step = UVAR('t') / 1000;
  const uint32_t t_step_us = UVAR('t');
  const xfloat k_all = 1e-6f * UVAR('v') / BOARD_ADC_DEFAULT_MAX;

  const uint32_t adc_arch_clock_in = ADC_getFreqIn( &adc.hadc );
  uint32_t s_div = 0;
  uint32_t div_bits = ADC_calc_div( &adc.hadc, BOARD_ADC_FREQ_MAX, &s_div );

  adc.set_channels( adc_channels );

  std_out <<  NL "# Test0: n= " << n << " n_ch= " << n_ch
    << " t= " << t_step << " ms, freq_in= " << adc_arch_clock_in
    << " freq_max= " << BOARD_ADC_FREQ_MAX << NL;

  if( s_div == 0  ||  div_bits == 0xFFFFFFFF ) {
    std_out << "# error: fail to calc divisor" NL;
    return 7;
  }
  const uint32_t adc_freq = adc_arch_clock_in / s_div;
  const uint32_t adc_arch_clock_ns = (unsigned)( ( 1000000000LL + adc_freq - 1 ) / adc_freq );
  std_out << "# div= " << s_div << " bits: " << HexInt( div_bits ) << " freq: " << adc_freq
          << " tau: " << adc_arch_clock_ns << NL;

  uint32_t unsigned stime_idx = ( (uint32_t)UVAR('s') < adc_arch_sampletimes_n ) ? UVAR('s') : (adc_arch_sampletimes_n - 1);
  uint32_t stime_ns =  adc_arch_clock_ns * ADC_conv_time_tick( stime_idx, n_ch, BOARD_ADC_DEFAULT_BITS );
  if( stime_ns == 0xFFFFFFFF ) {
    std_out << "# error: fail to calculate conversion time" NL;
    return 8;
  }

  if( t_step_us * 1000 <= stime_ns ) {
    std_out << "# warn: time step (" << t_step_us * 1000 << ") ns < conversion time (" << stime_ns << ") ns" NL;
  }

  std_out << "# stime_idx= " << stime_idx << " 10ticks= " << adc_arch_sampletimes[stime_idx].stime10
          << " stime_ns= "  << stime_ns  << " code= " <<  adc_arch_sampletimes[stime_idx].code << NL;

  adc.prepare_multi_softstart( div_bits, adc_arch_sampletimes[stime_idx].code, BOARD_ADC_DEFAULT_RESOLUTION );
  // adc.prepare_multi_softstart( div_bits, adc_arch_sampletimes[stime_idx].code, ADC_RESOLUTION_12B );

  if( ! adc.init_xxx1() ) {
    std_out << "# error: fail to init ADC: errno= " << errno << NL;
  }

  if( UVAR('d') > 1 ) {
    dump32( ADC1, 0x100 );
  }

  // or such
  ADC_freq_info fi;
  ADC_calcfreq( &adc.hadc, &fi );
  std_out << "# ADC: freq_in: " << fi.freq_in << " freq: " << fi.freq
          << " div: " << fi.div << " div1: " << fi.div1 << " div2: " << fi.div2
          << " bits: " << HexInt( fi.devbits ) << NL;

  // really need for H7 - DMA not work with ordinary memory
  uint16_t *ADC_buf = (uint16_t *) BOARD_ADC_MALLOC( n_ch * sizeof(uint16_t) );
  if( !ADC_buf ) {
    std_out << "# error: fail to alloc buffer" NL;
    return 13;
  }

  StatIntData sdat( n_ch, k_all );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR('b');


  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    float tc = 0.001f * ( tcc - tm00 );
    xfloat v[n_ch];

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    adc.end_dma = 0;

    for( decltype(+n_ch) i=0; i<n_ch; ++i ) {
      ADC_buf[i] = 0;
    }

    if( int sta = HAL_ADC_Start_DMA( &adc.hadc, (uint32_t*)(ADC_buf), n_ch ); sta != HAL_OK )   {
      std_out <<  "# error: ADC_Start_DMA error " << sta << NL;
      rc = 1;
      break;
    }

    for( uint32_t ti=0; adc.end_dma == 0 && ti<50000; ++ti ) { // 11
      delay_mcs( 2 );
    }

    HAL_ADC_Stop_DMA( &adc.hadc ); // needed
    if( UVAR('l') ) {  leds.reset( BIT2 ); }

    if( adc.end_dma == 0 ) {
      std_out <<  "# error: Fail to wait DMA end " NL;
      rc = 2;
      break;
    }

    if( adc.dma_error != 0 ) {
      std_out <<  "# error: Found DMA error " << HexInt( adc.dma_error ) <<  NL;
      rc = 3;
      break;
    } else {
      adc.n_series = 1;
    }

    if( do_out ) {
      std_out <<  FltFmt( tc, cvtff_auto, 12, 4 );
    }

    int vi[n_ch];
    for( decltype(n_ch) j=0; j<n_ch; ++j ) {
      vi[j] = ADC_buf[j];
      xfloat cv = k_all * ADC_buf[j] * (xfloat)v_coeffs[j];
      v[j] = cv;
    }
    sdat.add( vi ); // TODO: allow to add from uint16_t

    if( do_out ) {
      for( auto vc : v ) {
        std_out  << ' '  << XFmt( vc, cvtff_auto, 10, 6 );
      }
      // for( decltype(+n_ch) i=0; i<n_ch; ++i ) {
      //   std_out  << ' ' <<   HexInt( ADC_buf[i] );
      // }
      std_out << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  sdat.calc();
  std_out << sdat << NL;

  if( UVAR('d') > 1 ) {
    dump32( ADC1, 0x200 );
  }

  BOARD_ADC_FREE( ADC_buf );


  // HAL_ADC_Stop( &adc.hadc1 );

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

  // here?
  ADC_DMA_reinit( adc );

  HAL_NVIC_SetPriority( BOARD_ADC_DMA_IRQ, 2, 0 );
  HAL_NVIC_EnableIRQ(   BOARD_ADC_DMA_IRQ );

  // HAL_NVIC_SetPriority( BOARD_ADC_IRQ, 3, 0 );
  // HAL_NVIC_EnableIRQ( BOARD_ADC_IRQ );
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
  /* Invalidate Data Cache to get the updated content of the SRAM on the first half of the ADC converted data buffer: 32 bytes */
  // SCB_InvalidateDCache_by_Addr((uint32_t *) &aADCxConvertedData[0], ADC_CONVERTED_DATA_BUFFER_SIZE);
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT1 );
  // TODO: ADC_Info member
  adc.end_dma |= 1;
  // adc.good_SR =  adc.last_SR = adc.hadc.Instance->SR;
  adc.last_end = 1;
  adc.last_error = 0;
  ++adc.n_good;
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT0 );
  // TODO: ADC_Info member
  adc.end_dma |= 2;
  // adc.bad_SR = adc.last_SR = adc.hadc.Instance->SR;
  // tim2_deinit();
  adc.last_end  = 2;
  adc.last_error = HAL_ADC_GetError( hadc );
  adc.dma_error = hadc->DMA_Handle->ErrorCode;
  hadc->DMA_Handle->ErrorCode = 0;
  ++adc.n_bad;
}

void BOARD_ADC_DMA_IRQHANDLER(void)
{
  // leds.set( BIT2 );
  HAL_DMA_IRQHandler( &adc.hdma_adc );
}


// not used in single DMA
// void BOARD_ADC_IRQHANDLER(void)
// {
//   HAL_ADC_IRQHandler( &adc.hadc );
//   leds.toggle( BIT0 );
// }


int ADC_DMA_reinit( ADC_Info &adc )
{
  // std_out << "# debug: ADC_DMA_reinit start" NL;
  adc.hdma_adc.Instance                 = BOARD_ADC_DMA_INSTANCE;

  #ifdef BOARD_ADC_DMA_REQUEST
  adc.hdma_adc.Init.Request             = BOARD_ADC_DMA_REQUEST;
  #endif
  #ifdef BOARD_ADC_DMA_CHANNEL
  adc.hdma_adc.Init.Channel             = BOARD_ADC_DMA_CHANNEL;
  #endif

  adc.hdma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  adc.hdma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
  adc.hdma_adc.Init.MemInc              = DMA_MINC_ENABLE;
  adc.hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  adc.hdma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  adc.hdma_adc.Init.Mode                = DMA_NORMAL; // DMA_NORMAL, DMA_CIRCULAR, DMA_PFCTRL
  adc.hdma_adc.Init.Priority            = DMA_PRIORITY_HIGH; // DMA_PRIORITY_LOW, DMA_PRIORITY_MEDIUM, DMA_PRIORITY_HIGH, DMA_PRIORITY_VERY_HIGH
  adc.hdma_adc.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  // adc.hdma_adc.Init.FIFOThreshold    = DMA_FIFO_THRESHOLD_HALFFULL;
  // adc.hdma_adc.Init.MemBurst         = DMA_MBURST_SINGLE;
  // adc.hdma_adc.Init.PeriphBurst      = DMA_PBURST_SINGLE;

  HAL_DMA_DeInit( &adc.hdma_adc );
  if( HAL_DMA_Init( &adc.hdma_adc ) != HAL_OK ) {
    errno = 7777;
    return 0;
  }

  __HAL_LINKDMA( &adc.hadc, DMA_Handle, adc.hdma_adc );

  // std_out << "# debug: ADC_DMA_reinit end" NL;
  return 1;
}





int cmd_set_coeffs( int argc, const char * const * argv )
{
  if( argc > 1 ) {
    v_coeffs[0] = arg2float_d( 1, argc, argv, 1, -1e10f, 1e10f );
    v_coeffs[1] = arg2float_d( 2, argc, argv, 1, -1e10f, 1e10f );
    v_coeffs[2] = arg2float_d( 3, argc, argv, 1, -1e10f, 1e10f );
    v_coeffs[3] = arg2float_d( 4, argc, argv, 1, -1e10f, 1e10f );
  }
  std_out << "# Coefficients: "
     << v_coeffs[0] << ' ' << v_coeffs[1] << ' ' << v_coeffs[2] << ' ' << v_coeffs[3] << NL;
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

