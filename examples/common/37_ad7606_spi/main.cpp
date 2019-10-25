#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <oxc_ad7606_spi.h>


using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use AD7606 SPI ADC " NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] - test ADC input"  };
int cmd_set_coeffs( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_COEFFS { "set_coeffs", 'F', cmd_set_coeffs, " k0 k1 k2 k3 - set ADC coeffs"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SET_COEFFS,
  nullptr
};

const unsigned n_ADC_ch_max = 8;
float v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };


PinOut nss_pin(   BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS );
PinOut rst_pin(   BOARD_SPI_DEFAULT_GPIO_EXT1, BOARD_SPI_DEFAULT_GPIO_PIN_EXT1 );
PinOut cnvst_pin( BOARD_SPI_DEFAULT_GPIO_EXT2, BOARD_SPI_DEFAULT_GPIO_PIN_EXT2 );
PinsIn  busy_pin(              BOARD_IN0_GPIO,  BOARD_IN0_PINNUM, 1 );

SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );
AD7606_SPI adc( spi_d, rst_pin, cnvst_pin, busy_pin );

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') = 20;
  UVAR('c') = n_ADC_ch_max;
  UVAR('a') = 10; // aux delay

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_4 ) != HAL_OK ) {
    die4led( 0x04 );
  }

  adc.init();

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


#define DLY_T delay_mcs( 2 );

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int16_t ADC_buf[n_ADC_ch_max];
  unsigned n_ch = clamp( (unsigned)UVAR('c'), 1u, n_ADC_ch_max );
  int t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series
  bool do_out = ! UVAR('b');

  float kv = 5.0f / 0x7FFF; // TODO: adjust for real scope

  StatData sdat( n_ch );

  // if( UVAR('d') > 0 ) { // debug: for logic analizer start
  //   nss_pin.write( 0 );
  //   DLY_T;
  //   nss_pin.write( 1 );
  //   DLY_T;
  // }

  adc.reset();

  break_flag = 0;
  uint32_t tm0 = 0, tm00 = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    float tc = 0.001f * ( tcc - tm00 );
    sreal v[n_ch];

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    // adc.start();
    // auto nwait = adc.wait_nobusy();
    // auto rc = adc.read_only( ADC_buf, n_ch );
    // adc.reset();
    // delay_bad_mcs( 10 );
    auto rc = adc.read( ADC_buf, n_ch );
    if( UVAR('l') ) {  leds.reset( BIT2 ); }

    for( decltype(n_ch) j=0; j<n_ch; ++j ) {
      sreal cv = kv * ADC_buf[j]; // * v_coeffs[j];
      v[j] = cv;
    }
    sdat.add( v );

    if( do_out ) {
      std_out <<  FltFmt( tc, cvtff_auto, 12, 4 );

      for( auto vc : v ) {
        std_out  << ' '  <<  vc;
      }
      std_out << ' ' << rc << ' ' << HexInt16( ADC_buf[0] ) << ' ' << adc.get_busy_waited();
      std_out << NL;
    }

    if( t_step > 0 ) {
      delay_ms_until_brk( &tm0, t_step );
    } else {
      delay_bad_mcs( UVAR('a') );
    }
  }


  std_out << "# dt= " <<  ( 1e-3f * ( HAL_GetTick() - tm00 ) / n ) << NL;
  sdat.calc();
  std_out << sdat << NL;

  return 0;
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

