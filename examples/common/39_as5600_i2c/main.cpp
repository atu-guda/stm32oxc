#include <oxc_auto.h>

#include <oxc_as5600.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test AS5600 angle sensor" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 500;
  UVAR('n') = 50;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens;

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
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');

  std_out <<  NL "Test0: n= "  <<  n  <<  " t= "  <<  t_step <<  NL;

  ang_sens.setStartPosCurr();

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    auto alp_r = ang_sens.getAngle();
    uint32_t tcc = HAL_GetTick();
    auto alp_mDeg = AS5600::to_mDeg( alp_r );
    auto sta = ang_sens.getStatus();

    std_out <<  "i= "  <<  i  <<  "  tick: " <<  (tcc - tm00)
            << " alp_r= " << alp_r << " alp= " << FloatMult( alp_mDeg, 3 )
            << ' ' << HexInt8( sta ) << NL;

    std_out.flush();
    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

