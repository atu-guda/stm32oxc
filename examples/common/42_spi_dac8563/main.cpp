#include <oxc_auto.h>
#include <oxc_main.h>

#include <oxc_spi_dac8563.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test DAC563: 16-bit SPI DAC" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, "v0 v1 dv0 dv1 - test out. Uses n, t, a"  };
int cmd_send( int argc, const char * const * argv );
CmdInfo CMDINFO_SEND { "send", 'S', cmd_send, "cmd val - send one cmd+val"  };
int cmd_init( int argc, const char * const * argv );
CmdInfo CMDINFO_INIT { "init", 'I', cmd_init, " - defailt init"  };
int cmd_dacout( int argc, const char * const * argv );
CmdInfo CMDINFO_DACOUT { "dacout", 'D', cmd_dacout, "v16 [ch] - output"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SEND,
  &CMDINFO_INIT,
  &CMDINFO_DACOUT,
  nullptr
};



PinOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );
DevSPI_DAC8563 dac( spi_d );

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 1;

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_32 ) != HAL_OK ) {
    die4led( 0x04 );
  }
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
  uint16_t v0 = arg2long_d( 1, argc, argv, 0x7FFF, 0, 0xFFFF );
  uint16_t v1 = arg2long_d( 2, argc, argv, 0x7FFF, 0, 0xFFFF );
  int16_t dv0 = arg2long_d( 3, argc, argv, 0, -(0x7FFF), 0xFFFF );
  int16_t dv1 = arg2long_d( 4, argc, argv, 0, -(0x7FFF), 0xFFFF );
  uint32_t n  = UVAR('n');
  uint32_t t_step = UVAR('t');

  std_out << NL "# Test0: v0= " << v0  <<  " v1= " << v1 << " n= " << n
          << " dv0= " << dv0 << " dv1= " << dv1 << NL;

  break_flag = 0;
  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  for( uint32_t i=0; i<n && !break_flag; ++i ) {
    int ie = UVAR('a') ? (i&1) : i;
    uint16_t  v0c = v0 + ie * dv0;
    uint16_t  v1c = v1 + ie * dv1;
    dac.setu_a( v0c ); dac.setu_b( v1c );

    if( t_step > 0 ) {
      uint32_t tc = HAL_GetTick();
      if( t_step > 10 ) {
        std_out << i << ( tc - tm00 ) << ' ' << v0c << ' ' <<v1c << NL;
      }
      delay_ms_until_brk( &tm0, t_step );
    }
  }

  // near zero
  dac.setu_a( 0x8000 ); dac.setu_b( 0x8000 );

  return 0;
}

int cmd_send( int argc, const char * const * argv )
{
  uint8_t cmd = arg2long_d( 1, argc, argv, 0, 0, 0xFF );
  uint16_t  v = arg2long_d( 2, argc, argv, 0, 0, 0xFFFF );

  std_out << NL "# send: cmd= " << HexInt8(cmd)  <<  " v= " << HexInt16(v) << NL;

  auto rc = dac.write( cmd, v );

  std_out << "# rc= " << rc << NL;

  return 0;
}

int cmd_init( int argc, const char * const * argv )
{
  dac.init();
  return 0;
}


int cmd_dacout( int argc, const char * const * argv )
{
  uint16_t v = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );
  uint8_t ch = arg2long_d( 2, argc, argv, 0, 3, 3 );

  std_out << NL "# out: v= " << HexInt16(v) << " ch= " << ch << NL;

  auto rc = dac.setu( ch, v );

  std_out << "# rc= " << rc << NL;

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

