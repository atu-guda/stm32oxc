#include <cstring>
#include <ranges>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_atleave.h>
#include <oxc_floatfun.h>

#include <oxc_robo_base.h>

#include <oxc_gpio_pin_d.h>
#include <oxc_gpio_pins_d.h>

#include <oxc_tim_pwm_d.h>

#include <board_robo_cfg.h>

#include "main.h"


using namespace oxc;
using namespace SMLRL;
namespace ranges = std::ranges;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;


// auto out_q_fmt = [](xfloat x) { return FltFmt(x, cvtff_fix,8,4); };



int debug        {     0 };

TIM_HandleTypeDef  tim_servolwm_h;
Tim_Pwm_Dev        servolwm_hd( TIM_SERVOLWM_BASE, tim_SERVOLWM_chspins, tim_servolwm_h, 0xFFFF );
float              servolwm_io[3*6*3]; // TODO???
PwmRoboCapability  servolwm_rd( servolwm_hd, 3, servolwm_io, 600 );

TransFFLinLim tro_rad2pulse( 2.0e-3f/pi_f, 1.5e-3f, 0.5e-3f, 2.5e-3f ); // angle +/ pi/2 -> 500-2500 μs
OutChFF    oc_servolwm_rad2pulse0( servolwm_rd, PwmRoboCapability::ch0_pulse  , tro_rad2pulse );
OutChFF    oc_servolwm_rad2pulse1( servolwm_rd, PwmRoboCapability::ch0_pulse+1, tro_rad2pulse );
OutChFF    oc_servolwm_rad2pulse2( servolwm_rd, PwmRoboCapability::ch0_pulse+2, tro_rad2pulse );

IoRoboCapability* rcaps[] {
  &servolwm_rd,      // 0
};

RoboObject* robo_objs[] {
  &servolwm_rd,      // 0
};

OutChFBase* outchfs[] {
  &oc_servolwm_rad2pulse0, // 0
  &oc_servolwm_rad2pulse1, // 1
  &oc_servolwm_rad2pulse2, // 2
};

RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( robo_objs, robo_joints );

ReturnCode init_hw_all();

const char* common_help_string = "robo_lab02_ " __DATE__ " " __TIME__ NL;

// --- local commands;

// ------------------------   commands
DCL_CMD_REG( test0,       'T', " - test " );
DCL_CMD_REG( prt,         '\0', " - print timer info " );


void idle_main_task()
{
  robo.at_main_idle();
  // leds[1].toggle();
}

void on_sigint( int /* c */ )
{
  // tim_lwm_stop();
  break_flag = 1;
}

int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR_t =    50;
  UVAR_n =    1000;

  leds.initHW();   leds.reset( 0xFF_mask );

  if( ! init_hw_all().isOk() ) {
    std_out << "# Error: HW init" << NL;
    die4led( 1_mask );
  };

  // print_var_hook = print_var_ex;
  // set_var_hook   = set_var_ex;

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );
  dev_console.setOnSigInt( on_sigint );
  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  TIM_SERVOLWM_CLKEN();
  servolwm_hd.initHW();
  servolwm_hd.setFreq( SERVOLWM_FREQ );
  servolwm_hd.enable();
  return robo.init_all();
}




CMD_FUNCTION( test0 )
{
  float alp0  = arg2float_d( 1, argc, argv, 0,  -pi_f/2, pi_f/2 );
  float alp1  = arg2float_d( 2, argc, argv, 0,  -pi_f/2, pi_f/2 );
  oc_servolwm_rad2pulse0.out( alp0 );
  oc_servolwm_rad2pulse1.out( alp1 );
  // uint32_t t_step = arg2ulong_d( 2, argc, argv, 100,   2 );
  // default_loop( nn );
  robo.commit_all();
  return 0;
}

CMD_FUNCTION( prt )
{
  std_out << "# SERVOLWM: " NL;
  tim_print_cfg( TIM_SERVOLWM_BASE );
  return 0;
}


// ------------------------------------  ------------------------------------------------


