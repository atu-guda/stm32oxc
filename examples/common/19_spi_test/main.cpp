#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to manual control SPI devices" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_sendr_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_SENDR { "sendr", 'S', cmd_sendr_spi, "[0xXX ...] - send bytes, recv UVAR('r')"  };

int cmd_duplex_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_DUPLEX { "duplex", 'U', cmd_duplex_spi, "[0xXX ...] - send/recv bytes"  };

int cmd_recv_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_RECV { "recv", 'R', cmd_recv_spi, "[N] recv bytes"  };

int cmd_reset_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_RESETSPI { "reset_spi", 'Z', cmd_reset_spi, " - reset spi"  };

int cmd_sendloop_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_SENDLOOPSPI { "sendloop_spi", 'N', cmd_sendloop_spi, " N [val] - send N vals via SPI"  };

  const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SENDR,
  &CMDINFO_RECV,
  &CMDINFO_DUPLEX,
  &CMDINFO_SENDLOOPSPI,
  &CMDINFO_RESETSPI,
  nullptr
};



PinsOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 ); //  to test GPIO
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('r') = 0x20; // default bytes to read

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_256 ) != HAL_OK ) {
    die4led( 0x04 );
  }
  // nss_pin.initHW();
  //nss_pin.set(1);
  spi_d.setMaxWait( 500 );
  spi_d.initSPI();

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


#define DLY_T delay_mcs( 10 );

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint8_t sv = arg2long_d( 1, argc, argv, 0x15, 0, 0xFF );
  int nd     = arg2long_d( 2, argc, argv,    2, 0, sizeof(gbuf_a) );
  std_out << NL "Test0: sv= "  << HexInt8( sv ) << " nd= "  <<  nd  <<  NL;

  if( UVAR('d') > 0 ) { // debug: for logic analizer start
    nss_pin.write( 0 );
    DLY_T;
    nss_pin.write( 1 );
    DLY_T;
  }


  // spi_d.resetDev();

  int rc = spi_d.send_recv( sv, (uint8_t*)gbuf_a, nd );
  // int rc = spi_d.send( (uint8_t)sv );
  // int rc = spi_d.recv( (uint8_t*)gbuf_a, imin(UVAR('r'),sizeof(gbuf_a)) );


  std_out << "rc = " << rc << NL;
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  }
  spi_d.pr_info();

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
  std_out <<  NL "Send/recv: ns= "  <<  ns  <<  " nd= "  <<  nd  <<  "* to send: " NL;
  dump8( sbuf, ns );

  int rc = spi_d.send_recv( sbuf, ns, (uint8_t*)gbuf_a, nd );

  pr_sdx( rc );
  if( rc > 0 ) {
    std_out <<  "* recv: " NL ;
    dump8( gbuf_a, rc );
  } else {
    std_out <<  "** Error, code= "  << spi_d.getErr() <<  NL;
  }
  delay_ms( 10 );

  spi_d.pr_info();

  return 0;
}

int cmd_recv_spi( int argc, const char * const * argv )
{
  int nd = arg2long_d( 1, argc, argv, UVAR('r'), 1, sizeof(gbuf_a) );

  std_out <<  NL "Recv: nd= "  <<  nd  <<  NL;

  int rc = spi_d.recv( (uint8_t*)gbuf_a, nd );

  pr_sdx( rc );
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  } else {
    std_out <<  "** Error, code= "  << spi_d.getErr()<<  NL;
  }
  delay_ms( 10 );

  spi_d.pr_info();

  return 0;
}

int cmd_duplex_spi( int argc, const char * const * argv )
{
  uint8_t sbuf[16]; // really used not more then 9 - max args
  uint16_t ns = argc - 1;

  for( uint16_t i = 0; i<ns; ++i ) {
    uint8_t t = arg2long_d( i+1, argc, argv, 0, 0, 0xFF );
    sbuf[i] = t;
  }

  std_out <<  NL "Duplex: ns= "  <<  ns  <<  NL;
  dump8( sbuf, ns );

  int rc = spi_d.duplex( sbuf, (uint8_t*)gbuf_a, ns );

  std_out << "rc = " << rc << NL;
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  } else {
    std_out <<  "** Error, code= "  << spi_d.getErr() <<  NL;
  }
  delay_ms( 10 );

  spi_d.pr_info();

  return 0;
}

int cmd_sendloop_spi( int argc, const char * const * argv )
{
  int n      = arg2long_d( 1, argc, argv,    1, 1, 10000000 );
  uint8_t sv = arg2long_d( 2, argc, argv, 0x55, 0, 0xFF );
  std_out << NL "sendloop_spi: sv= "  << HexInt8( sv ) << " n= "  <<  n  <<  NL;
  for( int i=0; i<n; ++i ) {
    spi_d.send( sv );
  }
  return 0;
}


int cmd_reset_spi( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  spi_d.resetDev();

  spi_d.pr_info();

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

