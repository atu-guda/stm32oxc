#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <oxc_lsm303dlhc_accel.h>

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
LSM303DHLC_Accel accel( i2cd );

void MX_I2C1_Init( I2C_HandleTypeDef &i2c );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

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
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int scale = arg2long_d( 2, argc, argv, 0, 0, 3 );
  uint32_t t_step = UVAR('t');
  const uint8_t c4_scales[] = {
    LSM303DHLC_Accel::Ctl4_val::scale_2g,
    LSM303DHLC_Accel::Ctl4_val::scale_4g,
    LSM303DHLC_Accel::Ctl4_val::scale_8g,
    LSM303DHLC_Accel::Ctl4_val::scale_16g,
    0
  };

  uint8_t c4v = c4_scales[scale];
  if( argc > 3 ) {
    c4v |= (uint8_t)LSM303DHLC_Accel::Ctl4_val::hr_enable;
  }

  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( " scale= " ); pr_d( scale ); pr( " c4v= " ); pr_h( c4v );
  pr( NL );

  BarHText bar_x( 1, 1, 100, INT16_MIN, INT16_MAX );
  BarHText bar_y( 1, 2, 100, INT16_MIN, INT16_MAX );
  BarHText bar_z( 1, 3, 100, INT16_MIN, INT16_MAX );

  accel.resetDev();

  // if( ! accel.check_id() ) {
  //   pr( "LSM303DHLC_Accel no found" NL );
  //   return 1;
  // }
  if( ! accel.init( (LSM303DHLC_Accel::Ctl4_val)c4v ) ) {
    pr( "Fail to init LSM303DHLC_Accel" NL );
    return 1;
  }
  accel.rebootMem(); // bad try to drop previous measurement

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  int16_t xyz[3];

  // term_set_scroll_area( 2, 16 );
  // pr( NL );
  term_clear();
  // delay_ms( 50 );

  int el_t = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    accel.getAccAll( xyz );
    TickType_t tcc = xTaskGetTickCount(); el_t = tcc - tc00;
    bar_x.draw( xyz[0] );
    bar_y.draw( xyz[1] );
    bar_z.draw( xyz[2] );
    term_set_xy( 10, 5 );
    pr( "i= " ); pr_d( i );
    pr( "  tick: ");  pr_d( el_t );
    pr( " [ " ); pr_d( xyz[0] );
    pr( " ; " ); pr_d( xyz[1] );
    pr( " ; " ); pr_d( xyz[2] );
    pr( " ] " /* NL  */ );
    vTaskDelayUntil( &tc0, t_step );
  }

  // term_set_scroll_area( -1, -1 );

  pr( NL );
  pr_sdx( el_t );

  return 0;
}

int cmd_setaddr( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Need addr [1-127]" NL );
    return 1;
  }
  uint8_t addr  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   127 );
  accel.setAddr( addr );
  return 0;
}


//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

