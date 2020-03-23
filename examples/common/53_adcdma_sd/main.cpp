#include <cerrno>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_adcdata.h>
#include <oxc_adcdata_cmds.h>

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

// void pr_ADCDMA_state();

int adc_init_exa_4ch_dma( ADC_Info &adc, uint32_t adc_presc, uint32_t sampl_cycl, uint8_t n_ch );

const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX; // MCU dependent, in bytes for 16-bit samples
using AdcDataX = AdcData<12,xfloat>;
AdcDataX adcd( BOARD_ADC_MALLOC, BOARD_ADC_FREE );
const unsigned n_ADC_ch_max = 4;


uint32_t tim_freq_in; // timer input freq
int v_adc_ref = BOARD_ADC_COEFF; // in uV, measured before test, adjust as UVAR('v')



TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc = 36, uint32_t arr = 100 ); // 1MHz, 10 kHz
void tim2_deinit();

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
int cmd_outsd( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTSD { "outsd", 'X', cmd_outsd, "filename [N [start]]- output data to SD"  };
int cmd_outsdhex( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTSDHEX { "outsdhex", 'Z', cmd_outsdhex, "filename [N [start]]- output data to SD in hex"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SET_COEFFS,
  FS_CMDS0,
  &CMDINFO_OUT,
  &CMDINFO_OUTHEX,
  &CMDINFO_OUTSD,
  &CMDINFO_OUTSDHEX,
  &CMDINFO_SHOWSTATS,
  nullptr
};



int main(void)
{
  BOARD_PROLOG;

  // works bad with DMA
  #if defined(STM32F7)
  SCB_DisableICache();
  SCB_DisableDCache();
  #endif

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
  //
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)adc.n_ch_max );

  uint32_t tim_psc = UVAR('p');
  uint32_t tim_arr = UVAR('a');

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, n_ADC_series_max ); // number of series

  uint32_t sampl_t_idx = clamp( UVAR('s'), 0, (int)adc_n_sampl_times-1 );

  uint32_t t_step_tick =  (tim_arr+1) * (tim_psc+1); // in timer input ticks
  xfloat tim_f = (xfloat)tim_freq_in / t_step_tick; // timer update freq, Hz
  adc.t_step_f = (xfloat)t_step_tick / tim_freq_in; // in s
  uint32_t t_wait0 = 1 + uint32_t( n * adc.t_step_f * 1000 ); // in ms

  std_out << "# t_step_tick= " << t_step_tick << " [t2ticks] tim_f= " << tim_f << " Hz"
          << " t_step_f= " << adc.t_step_f << " s  t_wait0= " << t_wait0 << " ms" NL;

  if( n > n_ADC_series_max ) { n = n_ADC_series_max; };

  std_out << "# n= " << n << " n_ADC_series_max= " << n_ADC_series_max << NL;

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

  adcd.free();
  if( ! adcd.alloc( n_ch, n ) ) { // ?? + 2 is guard, may be remove
    std_out << "# Error: fail to alloc buffer" << NL;
    return 2;
  }
  adc.data = adcd.data();
  adc.reset_cnt();
  adcd.set_d_t( adc.t_step_f * 1e-6f );
  adcd.set_v_ref_uV( UVAR('v') );
  std_out << "# n_col= " << adcd.get_n_col() << " n_row= " << adcd.get_n_row() << " data: " << HexInt(adcd.data()) << " size_all= " << adcd.size_all() << NL;

  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  if( HAL_ADC_Start_DMA( &adc.hadc, (uint32_t*)adcd.data(), n_ADC_sampl ) != HAL_OK )   {
    std_out <<  "ADC_Start_DMA error" NL;
    return 10;
  }
  tim2_init( UVAR('p'), UVAR('a') );

  delay_ms_brk( t_wait0 );
  for( uint32_t ti=0; adc.end_dma == 0 && ti<(uint32_t)UVAR('t') && !break_flag;  ++ti ) {
    delay_ms( 1 );
  }
  tim2_deinit();
  HAL_ADC_Stop_DMA( &adc.hadc ); // needed

  uint32_t tcc = HAL_GetTick();
  delay_ms( 10 ); // to settle all

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

  // if( adc.n_series < 20 ) {
  //   adc_out_to( std_out, adc.n_series, 0 );
  // } else {
  //   adc_out_to( std_out, 4, 0 );
  //   std_out <<  "....." NL;
  //   adc_out_to( std_out, 4, adc.n_series-4 );
  // }

  std_out <<  NL;

  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }

  return 0;
}


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



int cmd_outsd( int argc, const char * const * argv )
{
  return subcmd_outsd_any( argc, argv, adcd, false );
}

int cmd_outsdhex( int argc, const char * const * argv )
{
  return subcmd_outsd_any( argc, argv, adcd, true );
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

