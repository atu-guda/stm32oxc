#include <oxc_auto.h>
#include <oxc_main.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to find and manual control I2C devices" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - detect I2C [start [end]]"  );


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 ); // zero add means no real device
I2CClient i2c_client( i2cd, 0x11 ); // fake but for client debug functions


int main(void)
{
  BOARD_PROLOG;

  UVAR_d =    1;
  UVAR_t = 1000;
  UVAR_n =   10;

  UVAR_e = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &i2c_client;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// better to use cmd_i2c_scan
// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int st_a = arg2long_d( 1, argc, argv,   2,    0, 126 );
  int en_a = arg2long_d( 2, argc, argv, 126, st_a, 126 );
  std_out << "Test0: st_a= " << st_a << " en_a= " << en_a << NL;
  std_out.flush();
  i2cd.resetDev();

  // uint8_t val;
  int i_err;
  for( uint8_t ad = (uint16_t)st_a; ad <= (uint16_t)en_a && ! break_flag; ++ad ) {
    bool rc = i2cd.ping( ad );
    i_err = i2cd.getErr();
    delay_ms( 10 );
    if( !rc ) {
      continue;
    }
    std_out << "ad = " << HexInt8( ad ) << "  rc= " << rc << "  err= " << i_err << NL;
    std_out.flush();
  }

  std_out << "# err= " << i2cd.getErr() << " state= " << i2cd.getState() << NL;

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

