#include <oxc_gpio.h>
#include <oxc_smallrl.h>
#include <oxc_common1.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace SMLRL;

void task_leds( void *prm UNUSED_ARG )
{
  while (1)
  {
    leds.toggle( BIT1 );
    delay_ms( 500 );
  }
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



void _exit( int rc )
{
  exit_rc = rc;
  die4led( rc );
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
  leds.toggle( BIT3 );
}

