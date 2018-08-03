#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_bmp085.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_setaddr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETADDR { "setaddr", 0, cmd_setaddr, " addr - set device addr (see 'C')"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SETADDR,
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


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  const int buf_sz = 80;
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');

  MSTRF( os, 128, prl1 );
  os << NL "Test0: n= " <<  n << " t= " << t_step << NL;
  os.flush();

  baro.readCalibrData();

  TickType_t tc0 = xTaskGetTickCount();

  char buf[buf_sz];
  int p_old = 0, p_00 = 0;

  for( int i=0; i<n && !break_flag ; ++i ) {
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
    ifcvt( t10, 10, buf, 1 );
    os << "T= " <<  buf << "  P= " << p << " dp= " << dp << " dp0= " << dp0 << NL;
    os.flush();
    delay_ms_until_brk( &tc0, t_step );
  }

  return 0;
}

int cmd_setaddr( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Need addr [1-127]" NL );
    return 1;
  }
  uint8_t addr  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   127 );
  baro.setAddr( addr );
  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

