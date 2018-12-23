#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cerrno>

#include <algorithm>
#include <vector>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_fs_cmd0.h>

#include <fatfs_sd_st.h>
#include <oxc_io_fatfs.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure ADC data (4ch) and store to SD card" NL;

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector
HAL_SD_CardInfoTypeDef cardInfo;
FATFS fs;
void out_to( OutStream &os, uint32_t n, uint32_t st );


int adc_init_exa_4ch_dma( ADC_Info &adc, uint32_t adc_presc, uint32_t sampl_cycl, uint8_t n_ch );

ADC_Info adc;

uint32_t tim_freq_in; // timer input freq
float t_step_f = 0.1; // in s, recalculated before measurement
int v_adc_ref = BOARD_ADC_COEFF; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX; // MCU dependent, in bytes for 16-bit samples

vector<uint16_t> ADC_buf;


TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc = 36, uint32_t arr = 100 ); // 1MHz, 10 kHz
void tim2_deinit();

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test ADC"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " [N [start]]- output data "  };
int cmd_outsd( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTSD { "outsd", 'X', cmd_outsd, "filename [N [start]]- output data to SD"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  FS_CMDS0,
  &CMDINFO_OUT,
  &CMDINFO_OUTSD,
  nullptr
};



int main(void)
{
  BOARD_PROLOG;

  tim_freq_in = get_TIM_in_freq( TIM2 ); // TODO: define

  // bsp_init_sdram( &hsdram );

  MX_SDIO_SD_Init(); // TODO: only during write or similar ops TODO: need generic commands rewrite
  UVAR('e') = HAL_SD_Init( &hsd );
  delay_ms( 10 );
  MX_FATFS_SD_Init();
  UVAR('x') = HAL_SD_GetState( &hsd ); // 0 = HAL_OK, 1 = HAL_ERROR, 2 = HAL_BUSY, 3 = HAL_TIMEOUT
  UVAR('y') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';
  UVAR('z') = f_mount( &fs, "", 1 );

  UVAR('t') = 1000; // 1 s extra wait
  UVAR('v') = v_adc_ref;
  UVAR('j') = tim_freq_in;
  UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM2, 1000000 ); // timer PSC, for 1MHz
  UVAR('a') = 99999; // timer ARR, for 10Hz TODO: better time or freq based
  UVAR('c') = n_ADC_ch_max;
  UVAR('n') = 8; // number of series
  UVAR('s') = 0; // sampling time index
  UVAR('d') = 1; // debug

  #ifdef PWR_CR1_ADCDC1
  PWR->CR1 |= PWR_CR1_ADCDC1;
  #endif

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}



// TODO: move
void pr_DMA_state()
{
  if( UVAR('d') > 0 ) {
    STDOUT_os;
    os <<  "# DMA: CR= " << HexInt( adc.hdma_adc.Instance->CR, true )
       << " NDTR= "      << adc.hdma_adc.Instance->NDTR
       << " PAR= "       << HexInt( adc.hdma_adc.Instance->PAR, true )
       << " M0AR= "      << HexInt( adc.hdma_adc.Instance->M0AR, true )
       << " M1AR= "      << HexInt( adc.hdma_adc.Instance->M1AR, true )
       << " FCR= "       << HexInt( adc.hdma_adc.Instance->FCR, true )
       << NL;
  }
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  STDOUT_os;
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)n_ADC_ch_max );

  uint32_t tim_psc = UVAR('p');
  uint32_t tim_arr = UVAR('a');

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, n_ADC_series_max ); // number of series

  uint32_t sampl_t_idx = clamp( UVAR('s'), 0, (int)adc_n_sampl_times-1 );
  uint32_t f_sampl_max = adc.adc_clk / ( sampl_times_cycles[sampl_t_idx] * n_ch );

  uint32_t t_step_tick =  (tim_arr+1) * (tim_psc+1); // in timer input ticks
  float tim_f = (float)tim_freq_in / t_step_tick; // timer update freq, Hz
  t_step_f    = (float)t_step_tick / tim_freq_in; // in s
  uint32_t t_wait0 = 1 + uint32_t( n * t_step_f * 1000 ); // in ms

  os << "# t_step_tick= " << t_step_tick << " [t2ticks] tim_f= " << tim_f << " Hz"
     << " t_step_f= " << t_step_f << " s  t_wait0= " << t_wait0 << " ms" NL;

  if( n > n_ADC_series_max ) { n = n_ADC_series_max; };

  tim2_deinit();

  uint32_t adc_presc = hint_ADC_presc();
  UVAR('i') =  adc_init_exa_4ch_dma( adc, adc_presc, sampl_times_codes[sampl_t_idx], n_ch );
  delay_ms( 1 );
  if( ! UVAR('i') ) {
    os << "ADC init failed, errno= " << errno << NL;
    return 1;
  }
  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }


  os << "# Timer: tim_freq_in= " << tim_freq_in << "  Hz / (( " << tim_psc
     << "+1)*( " << tim_arr << "+1)) =" << tim_f << " Hz; t_step = " << t_step_f << " s" NL;
  delay_ms( 1 );

  int div_val = -1;
  adc.adc_clk = calc_ADC_clk( adc_presc, &div_val );
  os << "# ADC: n_ch= " << n_ch << " n= " << n << " adc_clk= " << adc.adc_clk << " div_val= " << div_val
     << " s_idx= " << sampl_t_idx << " sampl= " << sampl_times_cycles[sampl_t_idx]
     << " f_sampl_max= " << f_sampl_max << " Hz" NL;
  delay_ms( 10 );

  uint32_t n_ADC_sampl = n * n_ch;
  ADC_buf.resize( 0, 0 );
  ADC_buf.shrink_to_fit();
  ADC_buf.assign( (n+2) * n_ch, 0 ); // + 2 is guard, may be remove
  os << "# ADC_buf.size= " << ADC_buf.size() << " data= " << HexInt( ADC_buf.data(), true ) << NL;

  adc.reset_cnt();
  if( ADC_buf.data() == nullptr ) {
    os <<  "# Error: fail to allocate memory" NL;
    return 2;
  }

  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  if( HAL_ADC_Start_DMA( &adc.hadc, (uint32_t*)ADC_buf.data(), n_ADC_sampl ) != HAL_OK )   {
    os <<  "ADC_Start_DMA error" NL;
  }
  tim2_init( UVAR('p'), UVAR('a') );

  delay_ms_brk( t_wait0 );
  for( uint32_t ti=0; adc.end_dma == 0 && ti<(uint32_t)UVAR('t') && !break_flag;  ++ti ) {
    delay_ms(1);
  }
  uint32_t tcc = HAL_GetTick();
  delay_ms( 10 ); // to settle all

  tim2_deinit();
  HAL_ADC_Stop_DMA( &adc.hadc ); // needed
  if( adc.end_dma == 0 ) {
    os <<  "Fail to wait DMA end " NL;
  }
  if( adc.dma_error != 0 ) {
    os <<  "Found DMA error " << HexInt( adc.dma_error ) <<  NL;
  } else {
    adc.n_series = n;
  }
  os <<  "#  tick: " <<  ( tcc - tm00 )  <<  NL;

  out_to( os, 2, 0 );
  if( adc.n_series > 2 ) {
    os <<  "....." NL;
    out_to( os, 4, adc.n_series-2 );
  }

  os <<  NL;

  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }
  os <<  NL;

  delay_ms( 10 );

  return 0;
}



void out_to( OutStream &os, uint32_t n, uint32_t st )
{
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)n_ADC_ch_max );

  if( n+st >= adc.n_series+1 ) {
    n = 1 + adc.n_series - st;
  }

  os << "# n= " << n << " n_ch= " << n_ch << " st= " << st << NL;

  float t = st * t_step_f;
  for( uint32_t i=0; i< n; ++i ) {
    uint32_t ii = i + st;
    t = t_step_f * ii;
    os << FloatFmt( t, "%#12.7g  " );
    for( int j=0; j< n_ch; ++j ) {
      float v = 0.001f * (float) ADC_buf[ii*n_ch+j] * UVAR('v') / 4096;
      os << FloatFmt( v, " %#10.6g" );
    }
    os << NL;
  }
}

int cmd_out( int argc, const char * const * argv )
{
  auto ns = adc.n_series;
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  STDOUT_os;
  out_to( os, n, st );

  return 0;
}

int cmd_outsd( int argc, const char * const * argv )
{
  STDOUT_os;
  if( argc < 2 ) {
    os << "# Error: need filename [n [start]]" NL;
    return 1;
  }

  uint32_t n = arg2long_d( 2, argc, argv, adc.n_series, 0, adc.n_series+1 ); // number output series
  uint32_t st= arg2long_d( 3, argc, argv,            0, 0, adc.n_series-2 );

  const char *fn = argv[1];
  auto file = DevOut_FatFS( fn );
  if( !file.isGood() ) {
    os << "Error: f_open error: " << file.getErr() << NL;
    return 2;
  }
  OutStream os_f( &file );
  out_to( os_f, n, st );

  return 0;
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  adc.end_dma |= 1;
  adc.good_SR =  adc.last_SR = adc.hadc.Instance->SR;
  adc.last_end = 1;
  adc.last_error = 0;
  ++adc.n_good;
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  adc.end_dma |= 2;
  adc.bad_SR = adc.last_SR = adc.hadc.Instance->SR;
  // tim2_deinit();
  adc.last_end  = 2;
  adc.last_error = HAL_ADC_GetError( hadc );
  adc.dma_error = hadc->DMA_Handle->ErrorCode;
  hadc->DMA_Handle->ErrorCode = 0;
  ++adc.n_bad;
}

void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler( &adc.hdma_adc );
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

