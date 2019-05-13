#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_bmp085.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to use BMP085 barometer" NL;

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
BMP085 baro( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &baro;

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

  std_out << NL "Test0: n= " << n << " t= " << t_step << NL;
  std_out.flush();

  baro.readCalibrData();

  // const int buf_sz = 80;
  // char buf[buf_sz];
  int p_old = 0, p_00 = 0;

  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    baro.getAllCalc( 3 );
    int t10 = baro.get_T10();
    int p   = baro.get_P();
    if( i == 0 ) {
      p_old = p_00 = p;
    }
    int dp   = p - p_old;
    int dp0  = p - p_00;
    p_old   = p;
    // int t_u = baro.get_T_uncons();
    // int p_u = baro.get_P_uncons();
    // ifcvt( t10, 10, buf, 1 );
    std_out << "T= " <<  FloatMult( t10, 1, 3 ) << "  P= " << p << " dp= " << dp << " dp0= " << dp0 << NL;

    std_out.flush();
    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

