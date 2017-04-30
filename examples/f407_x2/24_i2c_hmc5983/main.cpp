#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_hmc5983.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
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


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HMC5983 mag( i2cd );

void MX_I2C1_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 500;
  UVAR('n') = 50;

  MX_I2C1_Init( i2ch );
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
  constexpr auto n_scales = HMC5983::getNScales();
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int scale = arg2long_d( 2, argc, argv, 1, 0, n_scales-1 );
  uint32_t t_step = UVAR('t');

  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( " scale= " ); pr_d( scale );
  pr( NL );

  int scale_min = -1000000; // INT16_MIN
  int scale_max =  1000000; // INT16_MAX

  BarHText bar_x( 1, 1, 100, scale_min, scale_max );
  BarHText bar_y( 1, 2, 100, scale_min, scale_max );
  BarHText bar_z( 1, 3, 100, scale_min, scale_max );

  // mag.resetDev();

  if( ! mag.init(  HMC5983::cra_odr_75_Hz, HMC5983::Scales( scale ) ) ) {
    pr( "Fail to init HMC5983, Error= "  );
    pr_d( mag.getErr() ); pr( NL );
    return 1;
  }

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  int16_t temp;
  const int32_t *xyz;

  term_clear();

  for( int i=0; i<n && !break_flag; ++i ) {
    mag.read1( 10 );
    xyz = mag.getMagAllmcGa();
    temp = mag.getTemp();
    TickType_t tcc = xTaskGetTickCount();
    bar_x.draw( xyz[0] );
    bar_y.draw( xyz[1] );
    bar_z.draw( xyz[2] );
    term_set_xy( 10, 5 );
    pr( "i= " ); pr_d( i );
    pr( "  tick: "); pr_d( tcc - tc00 );
    pr( " [ " ); pr_d( xyz[0] );
    pr( " ; " ); pr_d( xyz[1] );
    pr( " ; " ); pr_d( xyz[2] );
    pr( " ] T= " /* NL  */ ); pr_d( temp ); pr( " " NL );
    vTaskDelayUntil( &tc0, t_step );
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
  mag.setAddr( addr );
  return 0;
}


//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

