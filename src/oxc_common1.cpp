#include <oxc_gpio.h>
#include <oxc_common1.h>
#include <oxc_outstream.h>

#include "board_cfg.h"

using namespace SMLRL;

#ifndef MAIN_LOOP_WAIT
#define MAIN_LOOP_WAIT 60000
#endif

#ifndef PS1_STRING
#define PS1_STRING "\033[32m#\033[0m "
#define PS1_OUTSZ  2
#endif

#if defined USE_FREERTOS

void task_leds( void *prm UNUSED_ARG )
{
  while (1) {
    int dly = task_leds_step * TASK_LEDS_QUANT;
    if( dly < 10 )      { dly =      10; }; // 10 ms min
    if( dly > 1000000 ) { dly = 1000000; }; // 1000 s max
    #ifdef LED_BSP_IDLE
      leds.toggle( LED_BSP_IDLE );
    #else
      #warning "LED_BSP_IDLE is not defined"
    #endif
    delay_ms( dly );
  }
  vTaskDelete(NULL);
}

void default_main_loop()
{
  int nl = 0;
  delay_ms( 10 );
  std_out <<  "###  " PROJ_NAME  " #### " NL;
  std_out.flush();
  delay_ms( 20 );
  if( ! global_smallrl ) {
    die( 0x01 );
  }

  reset_in( 0 );
  // global_smallrl->setSigFun( smallrl_sigint );
  global_smallrl->set_ps1( PS1_STRING, PS1_OUTSZ );
  global_smallrl->re_ps();
  global_smallrl->set_print_cmd( true );
  global_smallrl->reset();


  idle_flag = 1;
  while( 1 ) {
    ++nl;
    if( idle_flag == 0 ) {
      std_out << ".. main idle  " << nl;
      std_out.flush();
      global_smallrl->redraw();
    }
    idle_flag = 0;
    delay_ms( MAIN_LOOP_WAIT );
  }
}

void task_gchar( void *prm UNUSED_ARG )
{
  DevIO *cons = devio_fds[0];
  while( 1 ) {
    if( cons ) {
      auto v = cons->tryGet();
      if( v.good() ) {
        if( global_smallrl ) {
          global_smallrl->addChar( v.c );
        }
        idle_flag = 1;
      } else {
        delay_ms( 10 );
      }
    } else {
      delay_ms( 1000 );
    }
  }
  vTaskDelete(NULL);
}

#else

void led_task_nortos()
{
#ifdef LED_BSP_IDLE
  static OxcTicker led_tick( &task_leds_step, TASK_LEDS_QUANT );
  if( led_tick.isTick() ) {
    leds.toggle( LED_BSP_IDLE );
  }
#else
  #warning "LED_BSP_IDLE is not defined"
#endif
}

void std_main_loop_nortos( SmallRL *sm, AuxTickFun f_idle )
{
  if( !sm ) {
    die4led( 0 );
  }

  std_out <<  "###  " PROJ_NAME  " #### " NL;
  delay_ms( 10 );

  sm->reset();
  sm->re_ps();

  // eat pre-input
  reset_in( 0 );
  for( unsigned i=0; i<100; ++i ) {
    auto v = tryGet( 0 );
    if( ! v.good() ) {
      break;
    }
  }

  while( 1 ) {
    auto v = tryGet( 0 );

    if( v.good() ) {
      sm->addChar( v.c );
    } else {
      if( f_idle ) {
        f_idle();
      }
      delay_ms( 10 );
    }
  }
}

#endif
// FreeRTOS


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
  ++sigint_count;
  break_flag = 1;
  idle_flag = 1;
#if defined(LED_BSP_ERR)
  leds.toggle( LED_BSP_ERR );
#endif
}

