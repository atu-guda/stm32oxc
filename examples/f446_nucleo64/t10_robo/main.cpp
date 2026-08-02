#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>

#include <oxc_robo_base.h>

#include <oxc_gpio_d.h>

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

// ------------------------ Devices: capabilities ; ---------------------------------------

Gpio_Pin_Dev pin1_d( PC10 );

// ------------------------ - local sensors ; ---------------------------------------


// ------------------------ - local sensors end ---------------------------------------

TestRoboDevice test_rd{ 112 };


RoboObject* hw_robo_objs[] {
  &test_rd,
};


RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( hw_robo_objs, robo_joints );


void idle_main_task()
{
  robo.at_main_idle();
}


int main(void)
{
  BOARD_PROLOG;

  UVAR_l =    1; // idLe after run ?
  UVAR_n =   20; // n test

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
  pin1_d.initHW();

  return robo.init_all();
}

void test_pin1();

CMD_FUNCTION( test0 )
{
  test_pin1();

  // int v0 = arg2long_d( 2, argc, argv,  UVAR_v, INT_MIN, INT_MAX );

  return 0;
}


void test_pin1()
{
  switch( UVAR_z ) {
    case 0:
    for( int i=0; i<UVAR_n; ++i ) {
      pin1_d.set();
      delay_ms( 50 );
      std_out << pin1_d.read().value_or( 5 ) << NL;
      pin1_d.reset();
      delay_ms( 50 );
      std_out << pin1_d.read().value_or( 6 ) << NL;
    }
    break;

    case 1:
    for( int i=0; i<UVAR_n; ++i ) {
      pin1_d.toggle();
      delay_ms( 100 );
    }
    break;

    case 2:
    for( int i=0; i<UVAR_n; ++i ) {
      pin1_d.write( i & 1 );
      delay_ms( 200 );
    }
    break;

    case 3:
    for( int i=0; i<UVAR_n; ++i ) {
      pin1_d.setVal( 0, i & 1 );
      delay_ms( 100 );
    }
    break;
    default: break;
  }

  pin1_d.reset();
}
