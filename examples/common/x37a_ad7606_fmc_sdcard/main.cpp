#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_adcdata.h>
#include <oxc_adcdata_cmds.h>

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

const uint32_t n_ADC_mem  = BOARD_ADC_MEM_MAX_FMC; // MCU dependent, in bytes for 16-bit samples
using AdcDataX = AdcData<-16,xfloat>;
AdcDataX adcd( BOARD_ADC_MALLOC_EXT, BOARD_ADC_FREE_EXT );
const unsigned n_ADC_ch_max = 8;

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector
HAL_SD_CardInfoTypeDef cardInfo;
FATFS fs;


// --- local commands;
DCL_CMD_REG( test0, 'T', " [n] - test ADC input"  );
DCL_CMD_REG( set_coeffs, 'F', " k0 k1 k2 k3 - set ADC coeffs"  );
DCL_CMD_REG( out, 'O', " [N [start]]- output data "  );
DCL_CMD_REG( outhex, 'H', " [N [start]]- output data in hex form"  );
DCL_CMD_REG( show_stats, 'Y', " [N [start]]- show statistics"  );
DCL_CMD_REG( outsd, 'X', "filename [N [start]]- output data to SD"  );
DCL_CMD_REG( outsdhex, 'Z', "filename [N [start]]- output data to SD in hex"  );


PinOut nss_pin(   BOARD_SPI_DEFAULT_PIN_SNSS );
PinOut rst_pin(   BOARD_SPI_DEFAULT_PIN_EXT1 );
PinOut cnvst_pin( BOARD_SPI_DEFAULT_PIN_EXT2 );
PinsIn  busy_pin( BOARD_IN0, 1 );

SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );
AD7606_SPI adc( spi_d, rst_pin, cnvst_pin, busy_pin );

TIM_HandleTypeDef tim2h;
void tim2_init( uint16_t presc, uint32_t arr );
void tim2_deinit();
volatile unsigned tim_flag = 0;


int main(void)
{
  BOARD_PROLOG;

  bsp_init_sdram();


  UVAR_t = 20; // in us here
  UVAR_n = 100;
  UVAR_v = 5000000; // internal REF in uV
  UVAR_c = 4;

  MX_SDIO_SD_Init();

  UVAR_e = HAL_SD_Init( &hsd );
  delay_ms( 10 );
  MX_FATFS_SD_Init();
  UVAR_s = HAL_SD_GetState( &hsd );
  UVAR_z = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  fs.fs_type = 0; // none
  fspath[0] = '\0';

  if( SPI_init_default( BOARD_SPI_BAUDRATEPRESCALER_FAST ) != HAL_OK ) {
    die4led( 0x04_mask );
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
  int t_step = UVAR_t;
  unsigned n_ch = clamp( (unsigned)UVAR_c, 1u, n_ADC_ch_max );

  const uint32_t n_ADC_series_max  = n_ADC_mem / ( 2 * n_ch ); // 2 is 16bit/sample
  uint32_t n = arg2long_d( 1, argc, argv, UVAR_n, 1, n_ADC_series_max ); // number of series

  // int min_t_step = 50 * n_ch; // TMP
  // if( t_step < min_t_step ) {
  //   t_step = min_t_step;
  // }

  std_out << "# n = " << n << " n_ch= " << n_ch << " t_step= " << t_step << " us " NL;

  adc.reset();
  leds.set(   BIT0M | BIT1M | BIT2M ); delay_ms( 100 );  leds.reset( BIT0M | BIT1M | BIT2M );

  adcd.free();
  if( ! adcd.alloc( n_ch, n ) ) {
    std_out << "# Error: fail to alloc buffer" << NL;
    return 2;
  }
  adcd.set_d_t( t_step * 1e-6f );
  adcd.set_v_ref_uV( UVAR_v );

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

  leds[1].set();

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    while( tim_flag == 0 ) { /* NOP */ };
    tim_flag = 0;

    // if( UVAR_l ) {  leds[2].set(); }

    adc.read( adcd.row( i ), n_ch );

    // if( UVAR_l ) {  leds[2].reset(); }

  }

  HAL_NVIC_DisableIRQ( TIM2_IRQn );
  HAL_TIM_Base_Stop_IT( &tim2h );

  leds[1].reset();

  std_out << "# n_lines= " << adcd.get_n_row() << " dt_appr= " <<  ( 1e-3f * ( HAL_GetTick() - tm00 ) / n ) << NL;

  delay_ms( 10 );

  return rc;
}


int cmd_set_coeffs( int argc, const char * const * argv )
{
  return subcmd_set_coeffs( argc, argv, adcd );
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
    // TODO: Error_Handler( 4 );
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
  leds[1].toggle();
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

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

