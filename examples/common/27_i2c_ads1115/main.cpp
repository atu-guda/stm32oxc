#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_ads1115.h>

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


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
ADS1115 adc( i2cd );


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
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  int v = 0;
  uint16_t x_cfg = adc.getDeviceCfg();
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( "  cfg= " ); pr_h( x_cfg );
  pr( NL );
  bool is_cont = UVAR('c');

  adc.setDefault();

  UVAR('e') = adc.setCfg( ADS1115::cfg_in_0 | ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot );
  x_cfg = adc.getDeviceCfg();   pr( "  cfg= " ); pr_h( x_cfg );   pr( NL );

  if( is_cont ) {
    adc.startCont();
  }
  x_cfg = adc.getDeviceCfg();   pr( "  cfg= " ); pr_h( x_cfg );   pr( NL );

  TickType_t tc0 = xTaskGetTickCount();

  int scale_mv = adc.getScale_mV();
  pr_sdx( scale_mv );
  for( int i=0; i<n && !break_flag ; ++i ) {
    if( is_cont ) {
      v = adc.getContValue();
    } else {
      v = adc.getOneShot();
    }

    int xxv = scale_mv * v / 32678;
    pr( "[" ); pr_d( i ); pr( "]  " ); pr_d( v ); pr( "  xxv= " ); pr_d( xxv );
    pr( NL );
    vTaskDelayUntil( &tc0, t_step );
  }
  if( is_cont ) {
    adc.stopCont();
  }
  x_cfg = adc.getDeviceCfg();   pr( "  cfg= " ); pr_h( x_cfg );   pr( NL );

  return 0;
}

int cmd_setaddr( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Need addr [1-127]" NL );
    return 1;
  }
  uint8_t addr  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   127 );
  adc.setAddr( addr );
  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

