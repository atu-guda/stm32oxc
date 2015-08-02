#include <oxc_gpio.h>
#include <oxc_smallrl.h>
#include <oxc_common1.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace SMLRL;

volatile int task_leds_step = 5;

void task_leds( void *prm UNUSED_ARG )
{
  while (1)
  {
    int dly = task_leds_step * TASK_LEDS_QUANT;
    if( dly < 10 )    { dly = 10;    };
    if( dly > 10000 ) { dly = 10000; };
    leds.toggle( LED_BSP_IDLE );
    delay_ms( dly );
  }
  vTaskDelete(NULL);
}


void task_gchar( void *prm UNUSED_ARG )
{
  char sc[2] = { 0, 0 };
  while (1) {
    int n = recvByte( 0, sc, 10000 );
    if( n ) {
      if( global_smallrl ) {
        global_smallrl->addChar( sc[0] );
      }
      idle_flag = 1;
    }
  }
  vTaskDelete(NULL);
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

void sigint( int v UNUSED_ARG )
{
  smallrl_sigint();
}

void smallrl_sigint(void)
{
  break_flag = 1;
  idle_flag = 1;
  leds.toggle( LED_BSP_ERR );
}

