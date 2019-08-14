#include <oxc_auto.h>

#include <oxc_pca9685.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to control pca9685 I2C PWM" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_setFreq( int argc, const char * const * argv );
CmdInfo CMDINFO_SETFREQ { "setFreq", 'Q', cmd_setFreq, " freq - set frequency"  };
int cmd_setPulse( int argc, const char * const * argv );
CmdInfo CMDINFO_SETPULSE { "setPulse", 'U', cmd_setPulse, "n v  - set pulse value in us"  };
int cmd_setServo( int argc, const char * const * argv );
CmdInfo CMDINFO_SETSERVO { "setServo", 'E', cmd_setServo, "n v  - set servo value -1000:1000"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
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
  i2c_client_def = &pwmc;

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
  if( ! pwmc.init() ) {
    std_out <<  "Fail to init PCA9685, Error= " << pwmc.getErr() <<  NL;
    return 1;
  }

  int on   = arg2long_d( 1, argc, argv,    0, 0, 4095 );
  int off  = arg2long_d( 2, argc, argv, 2048, 0, 4095 );
  int puls = arg2long_d( 3, argc, argv, 1000, 0 );

  // pwmc.setFreq( 100 );
  // pwmc.setPresc( 3 );
  auto pre = pwmc.getPresc();
  auto freq = pwmc.getFreq();
  auto peri = pwmc.getPeriod_us();
  std_out << "presc= "   << pre << " freq= " << freq << " period= " << peri
     << " us, on= " << on  << " off= "  <<  off <<  NL;

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
  auto freq_get = pwmc.getFreq();
  auto peri = pwmc.getPeriod_us();
  std_out << "presc= " << pre << " freq= " << freq_get <<  " period= " <<  peri <<  NL;
  return 0;
}

int cmd_setPulse( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv, 0, 0, PCA9685::n_ch-1 );
  int v  = arg2long_d( 2, argc, argv, 0, 0 );
  uint16_t va = pwmc.us2v( v );
  std_out <<  "Pulse: n= " <<  n <<  " v= " <<  v << " va= " <<  va << NL;
  pwmc.set( n, 0, va );
  return 0;
}

int cmd_setServo( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv, 0, 0,  PCA9685::n_ch-1 );
  int v  = arg2long_d( 2, argc, argv, 0, -PCA9685::servo_in_max, PCA9685::servo_in_max );
  uint16_t va = pwmc.servo2v( v );
  uint16_t t  = pwmc.servo2t( v );
  std_out << "Pulse: n= " << n <<  " v= " << v <<  " va= " << va << " t= " << t << NL;
  pwmc.setServo( n, v );
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

