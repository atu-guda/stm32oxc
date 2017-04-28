#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_pcf8591.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;


const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
PCF8591 adc( &i2ch );

void MX_I2C1_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );


int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  MX_I2C1_Init( i2ch );
  i2c_dbg = &adc;

  delay_ms( PROLOG_LED_TIME ); leds.write( 0x01 ); delay_ms( PROLOG_LED_TIME );

  CREATE_STD_TASKS( task_usbcdc_send );

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
    // delay_ms( t_step );
  }
  adc.setOut( v_end );

  return 0;
}

//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

