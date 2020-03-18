#include <algorithm>
#include <vector>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
// #include <oxc_statdata.h>
#include <oxc_adcdata.h>

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
int cmd_outhex( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTHEX { "outhex", 'H', cmd_outhex, " [N [start]]- output data in hex form"  };
int cmd_show_stats( int argc, const char * const * argv );
CmdInfo CMDINFO_SHOWSTATS { "show_stats", 'Y', cmd_show_stats, " [N [start]]- show statistics"  };
int cmd_outsd( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTSD { "outsd", 'X', cmd_outsd, "filename [N [start]]- output data to SD"  };
int cmd_outsdhex( int argc, const char * const * argv );
CmdInfo CMDINFO_OUTSDHEX { "outsdhex", 'Z', cmd_outsdhex, "filename [N [start]]- output data to SD in hex"  };

int subcmd_outsd_any( int argc, const char * const * argv, bool isHex );
int subcmd_out_any( int argc, const char * const * argv, bool isHex );

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  FS_CMDS0,
  &CMDINFO_SET_COEFFS,
  &CMDINFO_OUT,
  &CMDINFO_OUTHEX,
  &CMDINFO_OUTSD,
  &CMDINFO_OUTSDHEX,
  &CMDINFO_SHOWSTATS,
  nullptr
};



PinOut nss_pin(   BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS );
PinOut rst_pin(   BOARD_SPI_DEFAULT_GPIO_EXT1, BOARD_SPI_DEFAULT_GPIO_PIN_EXT1 );
PinOut cnvst_pin( BOARD_SPI_DEFAULT_GPIO_EXT2, BOARD_SPI_DEFAULT_GPIO_PIN_EXT2 );
PinsIn  busy_pin(              BOARD_IN0_GPIO, BOARD_IN0_PINNUM, 1 );

SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );
AD7606_SPI adc( spi_d, rst_pin, cnvst_pin, busy_pin );

TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc, uint32_t arr );
void tim2_deinit();
volatile unsigned tim_flag = 0;

const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX_FMC; // MCU dependent, in bytes for 16-bit samples
AdcData<-16,xfloat> adcd( BOARD_ADC_MALLOC_EXT, BOARD_ADC_FREE_EXT );
using out_fun_type = decltype( &decltype(adcd)::out_hex );
const unsigned n_ADC_ch_max = 8;

int main(void)
{
  BOARD_PROLOG;

  bsp_init_sdram();


  UVAR('t') = 20; // in us here
  UVAR('n') = 100;
  UVAR('v') = 5000000; // internal REF in uV
  UVAR('c') = 4;

  MX_SDIO_SD_Init();

  UVAR('e') = HAL_SD_Init( &hsd );
  delay_ms( 10 );
  MX_FATFS_SD_Init();
  UVAR('s') = HAL_SD_GetState( &hsd );
  UVAR('z') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';

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

  adcd.free();
  if( ! adcd.alloc( n_ch, n ) ) {
    std_out << "# Error: fail to alloc buffer" << NL;
    return 2;
  }
  adcd.set_d_t( t_step * 1e-6f );
  adcd.set_v_ref_uV( UVAR('v') );

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

    adc.read( adcd.row( i ), n_ch );

    // if( UVAR('l') ) {  leds.reset( BIT2 ); }

  }

  HAL_NVIC_DisableIRQ( TIM2_IRQn );
  HAL_TIM_Base_Stop_IT( &tim2h );

  leds.reset( BIT1 );

  std_out << "# n_lines= " << adcd.get_n_row() << " dt_appr= " <<  ( 1e-3f * ( HAL_GetTick() - tm00 ) / n ) << NL;

  delay_ms( 10 );

  return rc;
}


int cmd_set_coeffs( int argc, const char * const * argv )
{
  if( argc > 1 ) {
    for( unsigned j=0; j< n_ADC_ch_max; ++j ) {
      xfloat v = arg2float_d( j+1, argc, argv, 1, -1e10f, 1e10f );
      adcd.set_col_mult( j, v );
    }
  }
  std_out << "# Coefficients:";
    for( unsigned j=0; j< n_ADC_ch_max; ++j ) {
    std_out << ' ' << adcd.get_col_mult( j );
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

int subcmd_out_any( int argc, const char * const * argv, bool isHex )
{
  auto ns = adcd.get_n_row();
  uint32_t n = arg2long_d( 1, argc, argv, ns, 0, ns+1 ); // number output series
  uint32_t st= arg2long_d( 2, argc, argv,  0, 0, ns-2 );

  adcd.out_any( std_out, isHex, st, n );

  return 0;
}

int cmd_out( int argc, const char * const * argv )
{
  return subcmd_out_any( argc, argv, false );
}

int cmd_outhex( int argc, const char * const * argv )
{
  return subcmd_out_any( argc, argv, true );
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

int subcmd_outsd_any( int argc, const char * const * argv, bool isHex )
{
  if( argc < 2 ) {
    std_out << "# Error: need filename [n [start]]" NL;
    return 1;
  }

  auto n_lines = adcd.get_n_row();
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
  adcd.out_any( os_f, isHex, st, n );
  StatIntData sdat( adcd );
  sdat.slurp( adcd );
  sdat.calc();
  os_f << sdat;
  leds.reset( BIT2 );

  return 0;
}


int cmd_outsd( int argc, const char * const * argv )
{
  return subcmd_outsd_any( argc, argv, false );
}

int cmd_outsdhex( int argc, const char * const * argv )
{
  return subcmd_outsd_any( argc, argv, true );
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

