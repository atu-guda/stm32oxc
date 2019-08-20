#include <cerrno>

#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

using namespace std;
using namespace SMLRL;

using sreal = StatData::sreal;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use MCP3204 ADC SPI device" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };


  const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};



PinsOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );

const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
float v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') = 10;
  UVAR('v') = 5100; // external power supply
  UVAR('c') = n_ADC_ch_max;

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_64 ) != HAL_OK ) { // 1.xx MBit/s < 2
    die4led( 0x04 );
  }
  // nss_pin.initHW();
  //nss_pin.set(1);
  spi_d.setMaxWait( 500 );
  // spi_d.setTssDelay( 10 );
  spi_d.initSPI();

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
  static const uint8_t mcp3204_cmds1ch[4][2] = {
    { 0x06, 0x00 },
    { 0x06, 0x40 },
    { 0x06, 0x80 },
    { 0x06, 0xC0 },
  };

  int t_step = UVAR('t');
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)n_ADC_ch_max );

  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  StatData sdat( n_ch );

  sreal kv = 0.001f * UVAR('v') / 4096;

  std_out << "# n = " << n << " n_ch= " << n_ch << " t_step= " << t_step << NL;
  std_out << "# Coeffs: ";
  for( decltype(n_ch) j=0; j<n_ch; ++j ) {
    std_out << ' ' << v_coeffs[j];
  }
  std_out << NL;

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0 = 0, tm00 = 0;
  int rc = 0;
  bool do_out = ! UVAR('b');

  uint16_t ADC_buf[n_ADC_ch_max];

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    float tc = 0.001f * ( tcc - tm00 );
    sreal v[n_ch];

    if( UVAR('l') ) {  leds.set( BIT2 ); }

    uint8_t buf[3]; // first - fake
    for( decltype(+n_ch) j=0; j < n_ch; ++j ) {
      spi_d.duplex( mcp3204_cmds1ch[j], buf, 3 );
      ADC_buf[j] = buf[2] + ( ( buf[1] & 0x0F ) << 8 );
      // delay_bad_mcs( 1 );
    }

    if( UVAR('l') ) {  leds.reset( BIT2 ); }

    for( decltype(n_ch) j=0; j<n_ch; ++j ) {
      sreal cv = kv * ADC_buf[j] * v_coeffs[j];
      v[j] = cv;
    }
    sdat.add( v );

    if( do_out ) {
      std_out <<  FltFmt( tc, cvtff_auto, 12, 4 );
      for( auto vc : v ) {
        std_out  << ' '  <<  vc;
      }
      std_out << NL;
    }

    if( t_step > 0 ) {
      delay_ms_until_brk( &tm0, t_step );
    }
  }


  std_out << "# dt= " <<  ( 1e-3f * ( HAL_GetTick() - tm00 ) / n ) << NL;
  sdat.calc();
  std_out << sdat << NL;

  delay_ms( 10 );

  return rc;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

