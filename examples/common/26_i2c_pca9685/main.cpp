#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_pca9685.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_setFreq( int argc, const char * const * argv );
CmdInfo CMDINFO_SETFREQ { "setFreq", 'Q', cmd_setFreq, " freq - set frequency"  };
int cmd_setPulse( int argc, const char * const * argv );
CmdInfo CMDINFO_SETPULSE { "setPulse", 'U', cmd_setPulse, "n v  - set pulse value in us"  };
int cmd_setServo( int argc, const char * const * argv );
CmdInfo CMDINFO_SETSERVO { "setServo", 'E', cmd_setServo, "n v  - set servo value -1000:1000"  };
int cmd_setaddr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETADDR { "setaddr", 0, cmd_setaddr, " addr - set device addr (see 'C')"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SETADDR,
  &CMDINFO_SETFREQ,
  &CMDINFO_SETSERVO,
  &CMDINFO_SETPULSE,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
PCA9685 pwmc( i2cd ); // 16-channel 12-bit PWM controller


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 500;
  UVAR('n') = 50;

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
  if( ! pwmc.init() ) {
    pr( "Fail to init PCA9685, Error= "  );
    pr_d( pwmc.getErr() ); pr( NL );
    return 1;
  }

  int on   = arg2long_d( 1, argc, argv,    0, 0, 4095 );
  int off  = arg2long_d( 2, argc, argv, 2048, 0, 4095 );
  int puls = arg2long_d( 3, argc, argv, 1000, 0 );

  // pwmc.setFreq( 100 );
  // pwmc.setPresc( 3 );
  auto pre = pwmc.getPresc();
  pr( "presc= " ); pr_d( pre );
  auto freq = pwmc.getFreq();
  pr( ", freq= " ); pr_d( freq );
  auto peri = pwmc.getPeriod_us();
  pr( ", period= " ); pr_d( peri );
  pr( " us, on= " );  pr_d( on );
  pr( ", off= " ); pr_d( off );
  pr( NL );

  // pwmc.setServo( 0, -50 );
  pwmc.set( 0, 0, pwmc.us2v( puls ) );
  pwmc.set( 1,  on, off );

  return 0;
}

int cmd_setFreq( int argc, const char * const * argv )
{
  int freq  = arg2long_d( 1, argc, argv,   50, 1 );
  pwmc.setFreq( freq );
  auto pre = pwmc.getPresc();
  pr( "presc= " ); pr_d( pre );
  auto freq_get = pwmc.getFreq();
  pr( ", freq= " ); pr_d( freq_get );
  auto peri = pwmc.getPeriod_us();
  pr( ", period= " ); pr_d( peri );
  pr( NL );
  return 0;
}

int cmd_setPulse( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv, 0, 0, PCA9685::n_ch-1 );
  int v  = arg2long_d( 2, argc, argv, 0, 0 );
  pr( "Pulse: n= " ); pr_d( n );
  pr( ", v= " ); pr_d( v );
  uint16_t va = pwmc.us2v( v );
  pr( ", va= " ); pr_d( va );
  pwmc.set( n, 0, va );
  pr( NL );
  return 0;
}

int cmd_setServo( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv, 0, 0,  PCA9685::n_ch-1 );
  int v  = arg2long_d( 2, argc, argv, 0, -PCA9685::servo_in_max, PCA9685::servo_in_max );
  pr( "Pulse: n= " ); pr_d( n );
  pr( ", v= " ); pr_d( v );
  uint16_t va = pwmc.servo2v( v );
  pr( ", va= " ); pr_d( va );
  uint16_t t = pwmc.servo2t( v );
  pr( ", t= " ); pr_d( t );
  pwmc.setServo( n, v );
  // pwmc.set( n, 0, va );
  pr( NL );
  return 0;
}

int cmd_setaddr( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Need addr [1-127]" NL );
    return 1;
  }
  uint8_t addr  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   127 );
  pwmc.setAddr( addr );
  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

