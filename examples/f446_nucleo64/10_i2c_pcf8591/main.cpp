#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_pcf8591.h>

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
PCF8591 adc( i2cd );

void MX_I2C1_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );


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
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( NL );

  int v_end = UVAR('e');

  adc.setMode( PCF8591::autoinc | PCF8591::mode_4in | PCF8591::out_en );

  TickType_t tc0 = xTaskGetTickCount();

  constexpr const int n_ch = 4;
  uint8_t d_in[n_ch] = { 0, 0, 0, 0 };

  for( int i=0; i<n && !break_flag ; ++i ) {

    adc.getIn( d_in, n_ch );
    pr( "[" ); pr_d( i ); pr( "]  " );
    for( int j=0; j<n_ch; ++j ) {
      pr_d( d_in[j] ); pr( "  " );
    }
    adc.setOut( i & 0xFF );
    pr( NL );
    vTaskDelayUntil( &tc0, t_step );
  }
  adc.setOut( v_end );

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


//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

