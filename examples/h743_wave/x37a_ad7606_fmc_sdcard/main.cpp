#include <algorithm>
#include <vector>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <oxc_fs_cmd0.h>

#include <oxc_ad7606_spi.h>

#include <fatfs_sd_st.h>
#include <oxc_io_fatfs.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use AD7606 SPI ADC with timer, SDRAM and SDIO" NL;

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector
HAL_SD_CardInfoTypeDef cardInfo;
FATFS fs;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] - test ADC input"  };
int cmd_set_coeffs( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_COEFFS { "set_coeffs", 'F', cmd_set_coeffs, " k0 k1 k2 k3 - set ADC coeffs"  };
int cmd_out( int argc, const char * const * argv );
CmdInfo CMDINFO_OUT { "out", 'O', cmd_out, " [N [start]]- output data "  };
int cmd_show_stats( int argc, const char * const * argv );
CmdInfo CMDINFO_SHOWSTATS { "show_stats", 'Y', cmd_show_stats, " [N [start]]- show statistics"  };
int cmd_outsd( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTSD { "outsd", 'X', cmd_outsd, "filename [N [start]]- output data to SD"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  FS_CMDS0,
  &CMDINFO_SET_COEFFS,
  &CMDINFO_OUT,
  &CMDINFO_OUTSD,
  &CMDINFO_SHOWSTATS,
  nullptr
};



PinOut nss_pin(   BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS );
PinOut rst_pin(   BOARD_SPI_DEFAULT_GPIO_EXT1, BOARD_SPI_DEFAULT_GPIO_PIN_EXT1 );
PinOut cnvst_pin( BOARD_SPI_DEFAULT_GPIO_EXT2, BOARD_SPI_DEFAULT_GPIO_PIN_EXT2 );
PinsIn  busy_pin(              BOARD_IN0_GPIO,  BOARD_IN0_PINNUM, 1 );

SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );
AD7606_SPI adc( spi_d, rst_pin, cnvst_pin, busy_pin );

TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc, uint32_t arr );
void tim2_deinit();
volatile unsigned tim_flag = 0;

const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX_FMC; // MCU dependent, in bytes for 16-bit samples
//vector<int16_t> ADC_buf;
int16_t *ADC_buf = (int16_t*)(SDRAM_BANK_ADDR);
unsigned n_lines = 0, last_n_ch = 1;
uint32_t last_dt_us = 1000;
const unsigned n_ADC_ch_max = 8;
float v_coeffs[n_ADC_ch_max];
void adc_out_to( OutStream &os, uint32_t n, uint32_t st );
void adc_show_stat( OutStream &os, uint32_t n, uint32_t st );
const unsigned adc_binmax = 0x7FFF;

int main(void)
{
  BOARD_PROLOG;

  bsp_init_sdram();


  UVAR('t') = 20; // in us here
  UVAR('n') = 100;
  UVAR('v') = 5000; // internal REF
  UVAR('c') = 4;

  MX_SDIO_SD_Init();

  UVAR('e') = HAL_SD_Init( &hsd );
  delay_ms( 10 );
  MX_FATFS_SD_Init();
  UVAR('s') = HAL_SD_GetState( &hsd );
  UVAR('z') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';

  for( auto &x : v_coeffs ) { x = 1.0f; };

  if( SPI_init_default( BOARD_SPI_BAUDRATEPRESCALER_FAST ) != HAL_OK ) {
    die4led( 0x04 );
  }
  spi_d.setTssDelay_100ns( 1 );

  adc.init();

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
  int t_step = UVAR('t');
  unsigned n_ch = clamp( (unsigned)UVAR('c'), 1u, n_ADC_ch_max );

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, n_ADC_series_max ); // number of series

  // int min_t_step = 50 * n_ch; // TMP
  // if( t_step < min_t_step ) {
  //   t_step = min_t_step;
  // }

  std_out << "# n = " << n << " n_ch= " << n_ch << " t_step= " << t_step << " us " NL;

  adc.reset();
  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  // ADC_buf.resize( 0, 0 );
  // ADC_buf.shrink_to_fit();
  // ADC_buf.assign( (n+2) * n_ch, 0 ); // + 2 is guard, may be remove
  n_lines = 0; last_n_ch = n_ch; last_dt_us = t_step;
  int16_t *buf_data = (int16_t*)(SDRAM_BANK_ADDR);

  uint32_t tm00 =  HAL_GetTick();
  int rc = 0;

  uint32_t psc = calc_TIM_psc_for_cnt_freq( tim2h.Instance, 1000000 ); // 1 us each
  tim2_init( psc, t_step - 1 );
  tim_print_cfg( tim2h.Instance );

  if( HAL_TIM_Base_Start_IT( &tim2h ) != HAL_OK ) {
    tim2_deinit();
    std_out << "Fail to init timer" NL;
    return 5;
  }
  HAL_NVIC_EnableIRQ( TIM2_IRQn );

  leds.set( BIT1 );

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    while( tim_flag == 0 ) { /* NOP */ };
    tim_flag = 0;

    // if( UVAR('l') ) {  leds.set( BIT2 ); }

    adc.read( buf_data + n_lines * n_ch, n_ch );

    // if( UVAR('l') ) {  leds.reset( BIT2 ); }

    ++n_lines;

  }

  HAL_NVIC_DisableIRQ( TIM2_IRQn );
  HAL_TIM_Base_Stop_IT( &tim2h );

  leds.reset( BIT1 );

  std_out << "# n_lines= " << n_lines << " dt_appr= " <<  ( 1e-3f * ( HAL_GetTick() - tm00 ) / n ) << NL;

  delay_ms( 10 );

  return rc;
}


int cmd_set_coeffs( int argc, const char * const * argv )
{
  if( argc > 1 ) {
    for( unsigned j=0; j< n_ADC_ch_max; ++j ) {
      v_coeffs[j] = arg2float_d( j+1, argc, argv, 1, -1e10f, 1e10f );
    }
  }
  std_out << "# Coefficients:";
  for( auto x: v_coeffs ) {
    std_out << ' ' << x;
  }
  std_out << NL;
  return 0;
}



void tim2_init( uint16_t presc, uint32_t arr )
{
  __TIM2_CLK_ENABLE();

  HAL_NVIC_SetPriority( TIM2_IRQn, 1, 0 );

  tim2h.Instance               = TIM2;
  tim2h.Init.Prescaler         = presc;
  tim2h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim2h.Init.Period            = arr;
  tim2h.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  tim2h.Init.RepetitionCounter = 0; // only for adv timers

  if( HAL_TIM_Base_Init( &tim2h ) != HAL_OK ) {
    Error_Handler( 4 );
    return;
  }

}

void tim2_deinit()
{
  HAL_NVIC_DisableIRQ( TIM2_IRQn );
  __TIM2_CLK_DISABLE();
}

void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &tim2h );
  tim_flag = 1;
  leds.toggle( BIT1 );
}


int cmd_out( int argc, const char * const * argv )
{
  auto ns = n_lines;
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  adc_out_to( std_out, n, st );
  adc_show_stat( std_out, n, st );

  return 0;
}


int cmd_show_stats( int argc, const char * const * argv )
{
  auto ns = n_lines;
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  adc_show_stat( std_out, n, st );

  return 0;
}

void adc_show_stat( OutStream &os, uint32_t n, uint32_t st )
{
  if( n+st >= n_lines+1 ) {
    n = n_lines - st;
  }

  StatData sdat( last_n_ch );

  for( uint32_t i=0; i< n; ++i ) {
    uint32_t ii = i + st;
    sreal vv[last_n_ch];
    for( decltype(+last_n_ch) j=0; j< last_n_ch; ++j ) {
      vv[j] = 0.001f * v_coeffs[j] * (float) ADC_buf[ii*last_n_ch+j] * UVAR('v') / adc_binmax;
    }
    sdat.add( vv );
  }
  sdat.calc();
  sdat.out_parts( os );

}

// some different from common ADC
void adc_out_to( OutStream &os, uint32_t n, uint32_t st )
{
  if( n+st >= n_lines ) {
    n = n_lines - st;
  }

  os << "# n= " << n << " n_ch= " << last_n_ch << " st= " << st
     << " dt= " << ( 1e-6f * last_dt_us ) << NL;

  break_flag = 0;
  for( uint32_t i=0; i<n && !break_flag; ++i ) {
    uint32_t ii = i + st;
    float t = 1e-6f * last_dt_us * ii;
    os <<  FltFmt( t, cvtff_auto, 14, 6 );
    for( decltype(+last_n_ch) j=0; j< last_n_ch; ++j ) {
      float v = 0.001f * v_coeffs[j] * (float) ADC_buf[ii*last_n_ch+j] * UVAR('v') / adc_binmax;
      os << ' ' << v;
    }
    os << NL;
  }

}

int cmd_outsd( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    std_out << "# Error: need filename [n [start]]" NL;
    return 1;
  }

  uint32_t n = arg2long_d( 2, argc, argv, n_lines, 0, n_lines+1 ); // number output series
  uint32_t st= arg2long_d( 3, argc, argv,       0, 0, n_lines-2 );

  const char *fn = argv[1];
  auto file = DevOut_FatFS( fn );
  if( !file.isGood() ) {
    std_out << "Error: f_open error: " << file.getErr() << NL;
    return 2;
  }

  leds.set( BIT2 );
  OutStream os_f( &file );
  adc_out_to( os_f, n, st );
  adc_show_stat( os_f, n, st );
  leds.reset( BIT2 );

  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

