#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>

// TMP: just to check header
#include <oxc_abstr_fun.h>

#include <oxc_robo_base.h>

#include <board_robo_cfg.h>


using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test misc robo parts. TMP." NL;



// ------------------------ - local commands; ---------------------------------------
DCL_CMD_REG(      test0,  'T',     " [arg ] - test something"  );

// -------------------------------------------------------------------------------------

ReturnCode init_hw_all();


// ------------------------ - local sensors ; ---------------------------------------


// ------------------------ - local sensors end ---------------------------------------

FakeRoboDevice fake_rd( "fake" );


RoboDevice* hw_robo_devs[] {
  &fake_rd,
};

RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( hw_robo_devs, robo_joints );


void idle_main_task()
{
  robo.at_main_idle();
}


int main(void)
{
  BOARD_PROLOG;

  UVAR_l =    1; // idLe after run ?

  if( ! init_hw_all().isOk() ) {
    std_out << "# Error: HW init" << NL;
    die4led( 1_mask );
  };

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  robo.start_time();

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  // q0 sens:

  return robo.init_all();
}



CMD_FUNCTION( test0 )
{
  // float pwm_v = arg2float_d( 1, argc, argv, 0.5f, 0.0f, 1.0f );
  // int v0 = arg2long_d( 2, argc, argv,  UVAR_v, INT_MIN, INT_MAX );

  return 0;
}

