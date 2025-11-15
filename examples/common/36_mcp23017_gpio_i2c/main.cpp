#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_mcp23017.h>

using namespace std;
using namespace SMLRL;


USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test MCP2017 GPIO I2C device" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test GPIO output"  );


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
MCP23017 mcp_gpio( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100; // 100 ms
  UVAR('n') = 1024;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &mcp_gpio;

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
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  std_out <<  NL "# Test0: n= " <<  n <<  " t= " <<  t_step << NL;

  mcp_gpio.cfg( MCP23017::iocon_intpol  );
  mcp_gpio.set_dir_a( 0x00 ); // all output
  mcp_gpio.set_dir_b( 0xFF ); // all input
  mcp_gpio.set_int_en_b( 0x01 ); // int enable

  uint32_t tm0, tm00;
  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }
    int dt = tcc - tm00; // ms

    mcp_gpio.set_a( (uint8_t)( i & 0xFF ) );
    uint8_t in = mcp_gpio.get_b();
    leds.sr( BIT1, in & 1 );

    std_out <<  dt << ' ' << (i & 0xFF) << ' ' << in << NL;

    delay_ms_until_brk( &tm0, t_step );
  }


  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

