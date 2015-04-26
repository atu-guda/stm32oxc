#include <cstring>
#include <cstdlib>


#include <bsp/board_stm32f407_atu_x2.h>
#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_debug_i2c.h>
#include <oxc_smallrl.h>

#include "usbd_desc.h"
#include <usbd_cdc.h>
#include <usbd_cdc_interface.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

UsbcdcIO usbcdc;


const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

// SmallRL storage and config
int smallrl_print( const char *s, int l );
int smallrl_exec( const char *s, int l );
void smallrl_sigint(void);
QueueHandle_t smallrl_cmd_queue;


SmallRL srl( smallrl_print, smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int idle_flag = 0;
int break_flag = 0;


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {

void task_main( void *prm UNUSED_ARG );
void task_leds( void *prm UNUSED_ARG );
void task_gchar( void *prm UNUSED_ARG );


}

I2C_HandleTypeDef i2ch;

STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );


  i2ch.Instance             = I2C1;
  i2ch.State                = HAL_I2C_STATE_RESET;
  i2ch.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  i2ch.Init.ClockSpeed      = 100000;
  i2ch.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  i2ch.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
  i2ch.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  i2ch.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  i2ch.Init.OwnAddress1     = 0;
  i2ch.Init.OwnAddress2     = 0;
  HAL_I2C_Init( &i2ch );
  i2ch_dbg = &i2ch;


  leds.write( 0x00 );

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_leds( void *prm UNUSED_ARG )
{
  while (1)
  {
    leds.toggle( BIT1 );
    delay_ms( 500 );
  }
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  usbcdc.init();
  delay_ms( 50 );
  user_vars['t'-'a'] = 1000;


  delay_ms( 10 );
  pr( "*=*** Main loop: ****** " NL );

  srl.setSigFun( smallrl_sigint );
  srl.set_ps1( "\033[32m#\033[0m ", 2 );
  srl.re_ps();
  srl.set_print_cmd( true );


  idle_flag = 1;
  while(1) {
    ++nl;
    if( idle_flag == 0 ) {
      pr_sd( ".. main idle  ", nl );
      srl.redraw();
    }
    idle_flag = 0;
    delay_ms( 60000 );
    // delay_ms( 1 );

  }
  vTaskDelete(NULL);
}

void task_gchar( void *prm UNUSED_ARG )
{
  char sc[2] = { 0, 0 };
  while (1) {
    int n = usbcdc.recvByte( sc, 10000 );
    if( n ) {
      // pr( NL "--- c='" ); pr( sc ); pr( "\"" NL );
      // leds.toggle( BIT0 );
      srl.addChar( sc[0] );
      idle_flag = 1;
    }
  }
  vTaskDelete(NULL);
}




void _exit( int rc )
{
  exit_rc = rc;
  die4led( rc );
}


int pr( const char *s )
{
  if( !s || !*s ) {
    return 0;
  }
  prl( s, strlen(s) );
  return 0;
}

int prl( const char *s, int l )
{
  // usbcdc.sendBlockSync( s, l );
  usbcdc.sendBlock( s, l );
  idle_flag = 1;
  return 0;
}

// ---------------------------- smallrl -----------------------


int smallrl_print( const char *s, int l )
{
  prl( s, l );
  return 1;
}

int smallrl_exec( const char *s, int l )
{
  exec_direct( s, l );
  return 1;
}


void smallrl_sigint(void)
{
  break_flag = 1;
  idle_flag = 1;
  leds.toggle( BIT3 );
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int st_a = 2;
  if( argc > 1 ) {
    st_a = strtol( argv[1], 0, 0 );
    pr( NL "Test0: argv[1]= \"" ); pr( argv[1] ); pr( "\"" NL );
  }
  int en_a = 127;
  if( argc > 2 ) {
    pr( NL "Test0: argv[2]= \"" ); pr( argv[2] ); pr( "\"" NL );
    en_a = strtol( argv[2], 0, 0 );
  }
  pr( NL "Test0: st_a= " ); pr_h( st_a ); pr( " en_a= " ); pr_h( en_a );
  pr( NL );

  // uint8_t val;
  int i_err;
  for( uint8_t ad = (uint16_t)st_a; ad <= (uint16_t)en_a && ! break_flag; ++ad ) {
    uint16_t ad2 = ad << 1;
    // HAL_StatusTypeDef rc = HAL_I2C_Master_Transmit( &i2ch, ad, &val, 1, 200 );
    HAL_StatusTypeDef rc = HAL_I2C_IsDeviceReady( &i2ch, ad2, 3, 200 );
    i_err = i2ch.ErrorCode;
    // pr( "ad = " ); pr_h( ad ); pr( "  rc= " ); pr_d( rc ) ; pr( "  err= " ); pr_d( i_err ); pr(NL);
    delay_ms( 10 );
    if( rc != HAL_OK ) {
      continue;
    }
    pr( "ad = " ); pr_h( ad ); pr( "  rc= " ); pr_d( rc ) ; pr( "  err= " ); pr_d( i_err ); pr(NL);
    // pr( NL "************************************ " );
    // pr_shx( ad );
  }


  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}



//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

