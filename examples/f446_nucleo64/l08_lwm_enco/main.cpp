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
DCL_CMD_REG( meas1,   'M', " lwm_us t_pre t_meas t_post - one measure"  );
DCL_CMD_REG( angle,   'A', " [set_val] - measure and ?set angle in ticks"  );


// auto out_v_fmt = [](xfloat x) { return FltFmt(x, cvtff_fix,8,4); };


void idle_main_task()
{
  // leds[1].toggle();
}


void set_lwm_from_us( uint32_t lwm_us );
int32_t go_measure( uint32_t lwm_us, int32_t t_pre, int32_t t_meas, int32_t t_post );
uint32_t tim_lwm_arr;


int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 4000; // measure time
  UVAR_w = 2000; // settle before measure
  UVAR_n =  100; // default main loop count
  UVAR_s =  500; // default start LWM us value
  UVAR_p = 2500; // default stop  LWM us value
  UVAR_s = 1000; // stop and wait after

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
  int n        = arg2long_d(   1, argc, argv, UVAR_n,  2, 10000 );
  int t_s      = arg2long_d(   2, argc, argv, UVAR_s,  1, 10000 );
  int t_e      = arg2long_d(   3, argc, argv, UVAR_p,  2, 10000 );

  int32_t t_meas = UVAR_t;
  int32_t t_pre    = UVAR_w;
  int32_t t_post = UVAR_s;

  int t_dlt = ( t_e - t_s ) / (n-1);


  std_out <<  "# Test0: n= " << n << " t_meas= " << t_meas << " t_pre= " << t_pre
          << " t_s= " << t_s << " t_e= " << t_e << " t_dlt " << t_dlt << NL;

  break_flag = 0;


  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t lwm = t_s + i * t_dlt;
    int32_t dn = go_measure( lwm, t_pre, t_meas, t_post );
    std_out << lwm << ' ' << dn << NL;
  }

  set_lwm_from_us( 0 );

  return 0;
}

int cmd_meas1( int argc, const char * const * argv )
{
  int32_t lwm     = arg2long_d(   1, argc, argv,      1500,   0, 10000 );
  int32_t t_pre   = arg2long_d(   2, argc, argv, UVAR_w,   0, 10000 );
  int32_t t_meas  = arg2long_d(   3, argc, argv, UVAR_t,   0, 10000 );
  int32_t t_post  = arg2long_d(   4, argc, argv, UVAR_s,  -1, 10000 );


  std_out <<  "# meas1: t_meas= " << t_meas << " t_pre= " << t_pre << " t_post " << t_post << NL;

  int32_t dn = go_measure( lwm, t_pre, t_meas, t_post );
  std_out << lwm << ' ' << dn << NL;

  return 0;
}

int cmd_angle( int argc, const char * const * argv )
{
  int32_t set_v    = arg2long_d(   1, argc, argv, -1 ); // -1 - do not set

  std_out <<  "# angle:= " << int(TIM_CNT->CNT) << NL;

  if( set_v != -1 ) {
    TIM_CNT->CNT = set_v;
  }

  return 0;
}

int32_t go_measure( uint32_t lwm_us, int32_t t_pre, int32_t t_meas, int32_t t_post )
{
  leds[0] = 1;
  set_lwm_from_us( lwm_us );

  if( delay_ms_brk( t_pre ) ) { // settle
    return 0;
  }
  leds[0] = 0;

  leds[1] = 1;
  TIM_CNT->CNT = 0;
  if( delay_ms_brk( t_meas ) ) { // measure
    return 0;
  }
  leds[1] = 0;
  int32_t dn = (int32_t)(TIM_CNT->CNT);

  if( t_post >= 0 ) {
    set_lwm_from_us( 0 );
    delay_ms_brk( t_post );
  }

  return dn;
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

