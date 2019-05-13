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

ADC_Info adc;

void adc_out_to( OutStream &os, uint32_t n, uint32_t st );
void adc_show_stat( OutStream &os, uint32_t n = 0xFFFFFFFF, uint32_t st = 0 );
void pr_ADCDMA_state();

int adc_init_exa_4ch_dma( ADC_Info &adc, uint32_t adc_presc, uint32_t sampl_cycl, uint8_t n_ch );


uint32_t tim_freq_in; // timer input freq
int v_adc_ref = BOARD_ADC_COEFF; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX; // MCU dependent, in bytes for 16-bit samples

vector<uint16_t> ADC_buf;


TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc = 36, uint32_t arr = 100 ); // 1MHz, 10 kHz
void tim2_deinit();

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test ADC"  };
int cmd_out( int argc, const char * const * argv );
extern CmdInfo CMDINFO_OUT;
int cmd_show_stats( int argc, const char * const * argv );
extern CmdInfo CMDINFO_SHOWSTATS;
int cmd_outsd( int argc, const char * const * argv );
extern CmdInfo CMDINFO_OUTSD;

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  FS_CMDS0,
  &CMDINFO_OUT,
  &CMDINFO_SHOWSTATS,
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
  const int base_freq = 1000000;
  UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM2, base_freq ); // timer PSC, for 1MHz
  UVAR('a') = ( base_freq / 10 ) - 1;
  UVAR('c') = 4; // n_ADC_ch_max;
  UVAR('n') = 8; // number of series
  UVAR('s') = 0; // sampling time index

  // TODO: test on F42x, F7xx
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



// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)adc.n_ch_max );

  uint32_t tim_psc = UVAR('p');
  uint32_t tim_arr = UVAR('a');

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, n_ADC_series_max ); // number of series

  uint32_t sampl_t_idx = clamp( UVAR('s'), 0, (int)adc_n_sampl_times-1 );

  uint32_t t_step_tick =  (tim_arr+1) * (tim_psc+1); // in timer input ticks
  float tim_f = (float)tim_freq_in / t_step_tick; // timer update freq, Hz
  adc.t_step_f = (float)t_step_tick / tim_freq_in; // in s
  uint32_t t_wait0 = 1 + uint32_t( n * adc.t_step_f * 1000 ); // in ms

  std_out << "# t_step_tick= " << t_step_tick << " [t2ticks] tim_f= " << tim_f << " Hz"
     << " t_step_f= " << adc.t_step_f << " s  t_wait0= " << t_wait0 << " ms" NL;

  if( n > n_ADC_series_max ) { n = n_ADC_series_max; };

  tim2_deinit();

  uint32_t adc_presc = hint_ADC_presc();
  UVAR('i') =  adc_init_exa_4ch_dma( adc, adc_presc, sampl_times_codes[sampl_t_idx], n_ch );
  delay_ms( 1 );
  if( ! UVAR('i') ) {
    std_out << "ADC init failed, errno= " << errno << NL;
    return 1;
  }
  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }


  std_out << "# Timer: tim_freq_in= " << tim_freq_in << "  Hz / ((" << tim_psc
     << "+1)*(" << tim_arr << "+1)) =" << tim_f << " Hz; t_step = " << adc.t_step_f << " s" NL;
  delay_ms( 1 );

  int div_val = -1;
  adc.adc_clk = calc_ADC_clk( adc_presc, &div_val );
  uint32_t f_sampl_max = adc.adc_clk / ( sampl_times_cycles[sampl_t_idx] * n_ch );
  std_out << "# ADC: n_ch= " << n_ch << " n= " << n << " adc_clk= " << adc.adc_clk << " div_val= " << div_val
     << " s_idx= " << sampl_t_idx << " sampl= " << sampl_times_cycles[sampl_t_idx]
     << " f_sampl_max= " << f_sampl_max << " Hz" NL;
  delay_ms( 10 );

  uint32_t n_ADC_sampl = n * n_ch;
  ADC_buf.resize( 0, 0 );
  ADC_buf.shrink_to_fit();
  ADC_buf.assign( (n+2) * n_ch, 0 ); // + 2 is guard, may be remove
  adc.data = ADC_buf.data();
  std_out << "# ADC_buf.size= " << ADC_buf.size() << " data= " << HexInt( ADC_buf.data(), true ) << NL;

  adc.reset_cnt();
  if( ADC_buf.data() == nullptr ) {
    std_out <<  "# Error: fail to allocate memory" NL;
    return 2;
  }

  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  if( HAL_ADC_Start_DMA( &adc.hadc, (uint32_t*)ADC_buf.data(), n_ADC_sampl ) != HAL_OK )   {
    std_out <<  "ADC_Start_DMA error" NL;
    return 10;
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
    std_out <<  "Fail to wait DMA end " NL;
  } else {
    if( adc.dma_error != 0 ) {
      std_out <<  "Found DMA error " << HexInt( adc.dma_error ) <<  NL;
    } else {
      adc.n_series = n;
    }
  }
  std_out <<  "#  tick: " <<  ( tcc - tm00 )  <<  NL;

  if( adc.n_series < 20 ) {
    adc_out_to( std_out, adc.n_series, 0 );
  } else {
    adc_out_to( std_out, 4, 0 );
    std_out <<  "....." NL;
    adc_out_to( std_out, 4, adc.n_series-4 );
  }

  std_out <<  NL;

  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

