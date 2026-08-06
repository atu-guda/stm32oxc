#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>

#include <oxc_robo_base.h>

#include <oxc_gpio_d.h>
#include <oxc_gpio_rd.h>

#include <board_robo_cfg.h>


using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test misc robo parts. TMP." NL;



// ------------------------ - local commands; ---------------------------------------
DCL_CMD_REG(      test_pin_d,   'T',     " [arg ] - pin1_d"  );
DCL_CMD_REG(      test_pin_rd,  'D',     " [arg ] - pin2_rd"  );
DCL_CMD_REG(      test_pins_d,  'S',     " [arg ] - pins_d"  );
DCL_CMD_REG(      test_pins_rd, 'U',     " [arg ] - pins_rd"  );

// -------------------------------------------------------------------------------------

ReturnCode init_hw_all();

// ------------------------ Devices: capabilities ; ---------------------------------------

Gpio_Pin_Dev pin1_d( PC10 );
Gpio_Pin_RDev pin2_rd( PC11 );

Gpio_Pins_Dev pins_d( PC0, 4 ); // copy of leds
Gpio_Pins_RDev pins_rd( PC0, 4 ); // copy of leds


// ------------------------ - local sensors ; ---------------------------------------


// ------------------------ - local sensors end ---------------------------------------

TestRoboDevice test_rd{ 112 };


RoboObject* hw_robo_objs[] {
  &test_rd,
  &pin2_rd,
  &pins_rd,
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

void test_pin1( uint32_t tp, uint32_t n );
void test_pin2( uint32_t tp, uint32_t n );
void test_pins_d( uint32_t tp, uint32_t n );
void test_pins_rd( uint32_t tp, uint32_t n );

int main(void)
{
  BOARD_PROLOG;

  UVAR_l =    1; // idle after run ?
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
  pin2_rd.initHW();

  pins_d.initHW(); // dup from leds, but may be another pins?

  return robo.init_all();
}


CMD_FUNCTION( test_pin_d )
{
  auto tp = arg2ulong_d( 1, argc, argv,  0 );
  auto n  = arg2ulong_d( 2, argc, argv,  UVAR_n );
  test_pin1( tp, n );

  return 0;
}


CMD_FUNCTION( test_pin_rd )
{
  auto tp = arg2ulong_d( 1, argc, argv,  0 );
  auto n  = arg2ulong_d( 2, argc, argv,  UVAR_n );
  test_pin2( tp, n );
  return 0;
}

CMD_FUNCTION( test_pins_d )
{
  auto tp = arg2ulong_d( 1, argc, argv,  0 );
  auto n  = arg2ulong_d( 2, argc, argv,  UVAR_n );
  test_pins_d( tp, n );
  return 0;
}

CMD_FUNCTION( test_pins_rd )
{
  auto tp = arg2ulong_d( 1, argc, argv,  0 );
  auto n  = arg2ulong_d( 2, argc, argv,  UVAR_n );
  test_pins_rd( tp, n );
  return 0;
}


void test_pin1( uint32_t tp, uint32_t n )
{
  switch( tp ) {
    case 0:
    for( uint32_t i=0; i<n; ++i ) {
      pin1_d.set();
      delay_ms( 50 );
      std_out << i << ' ' << pin1_d.read().value_or( 5 ) << ' ';
      pin1_d.reset();
      delay_ms( 50 );
      std_out << pin1_d.read().value_or( 6 ) << NL;
    }
    break;

    case 1:
    for( uint32_t i=0; i<n; ++i ) {
      pin1_d.toggle();
      delay_ms( 100 );
    }
    break;

    case 2:
    for( uint32_t i=0; i<n; ++i ) {
      pin1_d.write( i & 1 );
      delay_ms( 200 );
    }
    break;

    case 3:
    for( uint32_t i=0; i<n; ++i ) {
      pin1_d.setVal( 0, i & 1 );
      delay_ms( 100 );
    }
    break;
    default: break;
  }

  // pin1_d.reset();
}


void test_pin2( uint32_t tp, uint32_t n )
{
  pin2_rd.init();

  switch( tp ) {
    case 0:
    for( uint32_t i=0; i<n; ++i ) {
      pin2_rd.set(); pin2_rd.commit();
      delay_ms( 50 );
      pin2_rd.measure();
      std_out << i << ' ' << pin2_rd.read().value_or( 5 ) << ' ';
      pin2_rd.reset(); pin2_rd.commit();
      pin2_rd.measure();
      delay_ms( 50 );
      std_out << pin2_rd.read().value_or( 6 ) << NL;
    }
    break;

    case 1:
    for( uint32_t i=0; i<n; ++i ) {
      pin2_rd.toggle(); pin2_rd.commit();
      delay_ms( 100 );
    }
    break;

    case 2:
    for( uint32_t i=0; i<n; ++i ) {
      pin2_rd.write( i & 1 ); pin2_rd.commit();
      delay_ms( 200 );
    }
    break;

    case 3:
    for( uint32_t i=0; i<n; ++i ) {
      pin2_rd.setVal( 0, i & 1 ); pin2_rd.commit(); pin2_rd.measure();
      delay_ms( 100 );
      std_out << i << ' ' << (i&1) << ' ' << pin2_rd.read().value_or( 7 )
              << ' ' << pin2_rd.getValF( 0 ).value_or( 0.7f )
              << ' ' << pin2_rd.getValF( 1 ).value_or( 0.7f )
              << ' ' << pin2_rd.getValF( 2 ).value_or( 0.7f )
              << NL;
    }
    break;
    default: break;
  }

  // pin2_rd.reset(); pin2_rd.commit();
}

void test_pins_d( uint32_t tp, uint32_t n )
{
  for( uint32_t i=0; i<n; ++i ) {
    pins_d.write( i );
    delay_ms( 50 );
    std_out << pins_d.read().value_or( 255 ) << NL;
  }
}


void test_pins_rd( uint32_t tp, uint32_t n )
{
  for( uint32_t i=0; i<n; ++i ) {
    pins_rd.write( i ); pins_rd.toggle( 3 );
    pins_rd.commit(); pins_rd.measure();
    delay_ms( 50 );
    std_out << pins_rd.read().value_or( 255 ) << NL;
  }
}

