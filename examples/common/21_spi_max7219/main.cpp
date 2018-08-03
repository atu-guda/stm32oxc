#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_spi_max7219.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_sendr_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_SENDR { "sendr", 'S', cmd_sendr_spi, "[0xXX ...] - send bytes, recv UVAR('r')"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SENDR,
  nullptr
};



PinsOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );
DevSPI_MAX7219 max7219( spi_d );

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 20;
  UVAR('r') = 0; // default bytes to read

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_256 ) != HAL_OK ) {
  // if( SPI_init_default( SPI_BAUDRATEPRESCALER_32 ) != HAL_OK ) {
    die4led( 0x04 );
  }
  // nss_pin.initHW();
  //nss_pin.set(1);
  spi_d.initSPI();

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  default_main_loop();
  vTaskDelete(NULL);
}

#define DLY_T delay_mcs( 2 );

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  const int def_delay = 500;
  int v = arg2long_d( 1, argc, argv, 987 );
  uint8_t pos = arg2long_d( 2, argc, argv,  1, 0, 7 );
  uint8_t l = arg2long_d( 3, argc, argv,  3, 1, 8 );
  uint8_t v0[] = { 1, 2, 3, 4, 0x0E, 6, 7, 8 };
  pr( NL "Test0: v= " ); pr_d( v ); pr( " = " ); pr_h( v ); pr( "  pos= " ); pr_d( pos );
  pr( NL );

  if( UVAR('d') > 0 ) { // debug: for logic analizer start
    nss_pin.write( 0 );
    DLY_T;
    nss_pin.write( 1 );
    DLY_T;
  }


  max7219.setIntens( 1 );
  max7219.setDecode( 0xFE ); // position 0 - bitfield
  max7219.setLimit( 7 );

  for( uint8_t i=1; i<8; ++i ) {
    max7219.setDigit( i, i+8 );
  };
  max7219.setDigit( 0, 0xC9 );

  max7219.test_on();
  pr( "setDigit + test_on" NL ); delay_ms( def_delay );

  max7219.test_off();
  pr( "test_off" NL ); delay_ms( def_delay );

  max7219.off();
  pr( "off" NL ); delay_ms( def_delay );
  max7219.on();
  pr( "on" NL ); delay_ms( def_delay );

  max7219.setIntens( 3 );
  pr( "setIntens 3" NL ); delay_ms( def_delay );

  max7219.setIntens( 5 );
  pr( "setIntens 5" NL ); delay_ms( def_delay );

  max7219.setIntens( 7 );
  pr( "setIntens 7" NL ); delay_ms( def_delay );

  max7219.setIntens( 0 );
  pr( "setIntens 0" NL ); delay_ms( def_delay );

  max7219.setLimit( 4 );
  pr( "setLinit 4" NL ); delay_ms( def_delay );

  max7219.setLimit( 7 );
  pr( "setLinit 7" NL ); delay_ms( def_delay );

  max7219.setDecode( 0xFF ); // all = digits
  max7219.setDigits( v0, 3, 0, 5 );
  pr( "setDecode 0xFF, setDigits c0 3 0 5" NL ); delay_ms( def_delay );

  max7219.setUVal( 12345678, 7, 0, 8 );
  pr( "setUVal 12345678" NL ); delay_ms( def_delay );

  max7219.clsDig();
  pr( "clsDig " NL ); delay_ms( def_delay );

  max7219.setVal( v, 1, pos, l );

  delay_ms( def_delay );
  max7219.setDecode( 0x00 ); // all = bitmap
  max7219.setXVal( v, 1, 0, 8 );
  delay_ms( def_delay );
  max7219.setXDigit( 7, 0x10 );
  max7219.setXDigit( 6, 0x11 );
  max7219.setXDigit( 5, 0x12 );
  max7219.setXDigit( 4, 0x13 );
  max7219.setXDigit( 3, 0x14 );
  max7219.setXDigit( 2, 0x15 );

  delay_ms( 4 * def_delay );
  // max7219.setDecode( 0xFF ); // all = digits
  for( int i=0; i<8; ++i ) {
    max7219.setDigit( i, i );
  }

  delay_ms( 4 * def_delay );
  // max7219.setDecode( 0xFF ); // all = digits
  for( int i=0; i<8; ++i ) {
    max7219.setDigit( i, 1<<i | 1<<(i/2) | 0x81 );
  }

  return 0;
}

int cmd_sendr_spi( int argc, const char * const * argv )
{
  uint8_t sbuf[16]; // really used not more then 9 - max args
  uint16_t ns = argc - 1;

  for( uint16_t i = 0; i<ns; ++i ) {
    uint8_t t = arg2long_d( i+1, argc, argv, 0, 0, 0xFF );
    sbuf[i] = t;
  }

  int nd = imin( UVAR('r'), sizeof(gbuf_a) );
  pr( NL "Send/recv: ns= " ); pr_d( ns ); pr( " nd= " ); pr_d( nd );
  pr( NL );
  dump8( sbuf, ns );

  int rc = spi_d.send_recv( sbuf, ns, (uint8_t*)gbuf_a, nd );

  pr_sdx( rc );
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  }

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

