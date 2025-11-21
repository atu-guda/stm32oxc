#include <cstring>
#include <cerrno>
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>

#include "main.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure fb-less serwo speed" NL;


// --- local commands;
DCL_CMD_REG( test0,   'T', "n t_s t_e - test "  );
DCL_CMD_REG( timinfo, 'I', " - timers info"  );
DCL_CMD_REG( set_lwm, 'L', " us - set LWM"  );


// auto out_v_fmt = [](xfloat x) { return FltFmt(x, cvtff_fix,8,4); };


void idle_main_task()
{
  // leds.toggle( 1 );
}


void set_lwm_from_us( uint32_t lwm_us );
uint32_t tim_lwm_arr;


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 4000; // measure time
  UVAR('w') = 2000; // settle before measure
  UVAR('n') =  100; // default main loop count
  UVAR('s') =  500; // default start LWM us value
  UVAR('p') = 2500; // default stop  LWM us value
  UVAR('x') =    1; // stop after each measurement

  MX_TIM_CNT_Init();
  HAL_TIM_Base_Start( &htim_cnt );
  MX_TIM_LWM_Init();
  HAL_TIM_PWM_Start( &htim_lwm, TIM_LWM_CHANNEL );

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n        = arg2long_d(   1, argc, argv, UVAR('n'),  2, 10000 );
  int t_s      = arg2long_d(   2, argc, argv, UVAR('s'),  1, 10000 );
  int t_e      = arg2long_d(   3, argc, argv, UVAR('p'),  2, 10000 );

  uint32_t t_meas = UVAR('t');
  uint32_t t_w    = UVAR('w');

  int t_dlt = ( t_e - t_s ) / (n-1);


  std_out <<  "# Test0: n= " << n << " t_meas= " << t_meas << " t_w= " << t_w
          << " t_s= " << t_s << " t_e= " << t_e << " t_dlt " << t_dlt << NL;

  break_flag = 0;


  for( int i=0; i<n && !break_flag; ++i ) {

    leds[0] = 1;
    uint32_t lwm = t_s + i * t_dlt;
    set_lwm_from_us( lwm );

    if( delay_ms_brk( t_w ) ) { // settle
      break;
    }
    leds[0] = 0;

    leds[1] = 1;
    TIM_CNT->CNT = 0;
    if( delay_ms_brk( t_meas ) ) { // measure
      break;
    }
    leds[1] = 0;
    int32_t dn = (int32_t)(TIM_CNT->CNT);

    if( UVAR('x') ) {
      set_lwm_from_us( 0 );
    }
    delay_ms_brk( 1000 );
    std_out << lwm << ' ' << dn << NL;

  }

  break_flag = 0;
  set_lwm_from_us( 0 );

  return 0;
}


void set_lwm_from_us( uint32_t lwm_us )
{
  const uint32_t ccr = (uint32_t)( lwm_us * 1000L / tim_lwm_dt_ns ); // really *2, but for outher freqs...
  TIM_LWM->LWM_CCR = ccr;
}

int cmd_set_lwm( int argc, const char * const * argv )
{
  uint32_t lwm_us = arg2long_d(   1, argc, argv, 1500,  0, 3000 );
  set_lwm_from_us( lwm_us );

  std_out << "# LWM: " << lwm_us << " us, ccr= " << TIM_LWM->LWM_CCR << NL;

  return 0;
}



int cmd_timinfo( int argc, const char * const * argv )
{
  tim_print_cfg( TIM_CNT );
  tim_print_cfg( TIM_LWM );
  return 0;
}

// ------------------------------------------------------------ 



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

