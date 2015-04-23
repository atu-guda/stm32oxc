#include <cstring>
#include <cstdlib>


#include <bsp/board_stm32f407_atu_x2.h>
#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_smallrl_q.h>

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


// SmallRL srl( smallrl_print, smallrl_exec );
SmallRL srl( smallrl_print, exec_queue );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int idle_flag = 0;
int break_flag = 0;


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {

void task_main( void *prm UNUSED_ARG );
void task_leds( void *prm UNUSED_ARG );


}

void on_received_char( const char *s, int l );

// STD_USART2_IRQ( usartio );
STD_USBCDC_RECV_TASK( usbcdc );
STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );

  //usbcdc.init();
  leds.write( 0x07 );  delay_bad_ms( 200 );


  leds.write( 0x00 );

  global_smallrl = &srl;
  SMALLRL_INIT_QUEUE;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_usbcdc_recv, "recv", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_smallrl_cmd, "scmd", 2*def_stksz, nullptr,   1, nullptr );

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

extern uint8_t UserTxBuffer[];

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  usbcdc.init();
  delay_ms( 50 );
  user_vars['t'-'a'] = 1000;

  usbcdc.setOnRecv( on_received_char );

  delay_ms( 10 );
  pr( "*=*** Main loop: ****** " NL );

  srl.setSigFun( smallrl_sigint );
  srl.set_ps1( "\033[32m#\033[0m ", 2 );
  srl.re_ps();


  idle_flag = 1;
  while (1) {
    // usbcdc.sendBlockSync( "<.ABCDEFGHIJKLMNOPQRSTUVW..XYZ01234567890>\r\n" , 44 );
    // delay_ms(1);
    // usbcdc.sendBlockSync( "<!ZBCDEFGHIJKLMNOPQRSTUVW..XYZ01234567890>\r\n" , 44 );
    // delay_ms(1);
    // usbcdc.sendBlock(     "<:abcdefghijklmnopqrstuvw..xyz01234567890>\r\n" , 44 );
    // delay_ms(1);
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
  pr( NL "Cmd: \"" );
  prl( s, l );
  pr( "\" " NL );
  exec_direct( s, l );
  return 1;
}

// will be called by real receiver: USART, USB...
void on_received_char( const char *s, int l )
{
  // leds.toggle( BIT2 );
  if( !s || l<1 ) { return; }
  for( int i=0; i<l; ++i ) {
    srl.addChar( *s++ );
  }
  idle_flag = 1;
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
  int a1 = 0;
  if( argc > 1 ) {
    a1 = strtol( argv[1], 0, 0 );
  }
  pr( NL "Test0: a1= " ); pr_d( a1 );
  pr( NL );

  int prty = uxTaskPriorityGet( 0 );
  pr_sdx( prty );
  const char *nm = pcTaskGetTaskName( 0 );
  pr( "name: \"" ); pr( nm ); pr( "\"" NL );

  // log_add( "Test0 " );
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  uint32_t t_step = user_vars['t'-'a'];
  uint32_t tm0 = HAL_GetTick();

  for( int i=0; i<16 && !break_flag; ++i ) {
    TickType_t tcc = xTaskGetTickCount();
    uint32_t tmc = HAL_GetTick();
    pr( " Fake Action i= " ); pr_d( i );
    pr( "  tick: "); pr_d( tcc - tc00 );
    pr( "  ms_tick: "); pr_d( tmc - tm0 );
    pr( NL );
    vTaskDelayUntil( &tc0, t_step );
    // delay_ms( 1000 );
    // MillisecondTimer::delay(1000);
  }

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;

  pr( NL "test0 end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

