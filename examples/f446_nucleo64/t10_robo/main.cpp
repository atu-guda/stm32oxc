#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>

#include <oxc_robo_base.h>

#include <oxc_gpio_d.h>

#include <oxc_tim_pwm_d.h>

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

Gpio_Pin_Dev  pin1_hd( PC10 );
PinCapability pin1_d( pin1_d );
Gpio_Pin_Dev  pin2_hd( PC11 );
PinRoboCapability pin2_rd( pin2_hd );
Gpio_Pin_Dev  pini_hd( PC13 );
PinRoboCapability pini_rd( pini_hd );

Gpio_Pins_Dev pins_hd( PC0, 4 ); // copy of leds
PinsCapability pins_d( pins_hd );
PinsRoboCapability pins_rd( pins_hd ); // copy of leds


// ------------------------ - local sensors ; ---------------------------------------


// ------------------------ - local sensors end ---------------------------------------

// TestRoboDevice test_rd{ 112 };


RoboObject* hw_robo_objs[] {
  // &test_rd,
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
  UVAR_s = 1000; // scale
  UVAR_t =  100; // default delay

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
  //pin1_d.initHW();
  //pin2_rd.initHW();
  // pini_rd.getPin()->cfgIn();
  PC13.cfgIn();

  //pins_d.initHW(); // dup from leds, but may be another pins?

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
      pin1_hd.set();
      delay_ms( UVAR_t );
      std_out << i << ' ' << pin1_hd.read().value_or( 5 ) << ' ';
      pin1_hd.reset();
      delay_ms( UVAR_t );
      std_out << pin1_hd.read().value_or( 6 ) << NL;
    }
    break;

    case 1:
    for( uint32_t i=0; i<n; ++i ) {
      pin1_hd.toggle();
      delay_ms( 100 );
    }
    break;

    case 2:
    for( uint32_t i=0; i<n; ++i ) {
      pin1_hd.write( i & 1 );
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
      pin2_hd.set(); pin2_rd.commit();
      delay_ms( UVAR_t );
      pin2_rd.measure();
      std_out << i << ' ' << pin2_hd.read().value_or( 5 ) << ' ';
      pin2_hd.reset(); pin2_rd.commit();
      pin2_rd.measure(); pini_rd.measure();
      delay_ms( UVAR_t );
      std_out << pin2_hd.read().value_or( 6 )
        << ' ' << pini_rd.getVal( 0 /*PinCapability::ch_in*/ ).value_or( 77 ) << NL;
    }
    break;

    case 1:
    for( uint32_t i=0; i<n; ++i ) {
      pin2_hd.toggle(); pin2_rd.commit();
      delay_ms( 100 );
    }
    break;

    case 2:
    for( uint32_t i=0; i<n; ++i ) {
      pin2_hd.write( i & 1 ); pin2_rd.commit();
      delay_ms( 200 );
    }
    break;

    case 3:
    for( uint32_t i=0; i<n; ++i ) {
      pin2_rd.setVal( 0, i & 1 ); pin2_rd.commit(); pin2_rd.measure();
      delay_ms( 100 );
      std_out << i << ' ' << (i&1) << ' ' << pin2_hd.read().value_or( 7 )
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
    pins_hd.write( i );
    delay_ms( UVAR_t );
    std_out << pins_hd.read().value_or( 255 ) << NL;
  }
}


void test_pins_rd( uint32_t tp, uint32_t n )
{
  switch( tp ) {
    case 0:
      for( uint32_t i=0; i<n; ++i ) {
        pins_hd.write( i ); pins_hd.toggle( 3 );
        pins_rd.commit(); pins_rd.measure();
        delay_ms( UVAR_t );
        std_out << pins_hd.read().value_or( 255 ) << NL;
      }
      break;

    case 1:
      for( uint32_t i=0; i<n; ++i ) {
        float v = 0.1 * i * UVAR_s;
        pins_rd.setValF( 0, v ); pins_rd.commit(); pins_rd.measure();
        delay_ms( UVAR_t );
        std_out << v << ' ' << pins_hd.read().value_or( 255 )
          << ' ' << pins_rd.getVal( 0 ).value_or( -555 )
          << ' ' << pins_rd.getValF( 0 ).value_or( -555.0f )
          << NL;
      }
      break;

    default: break;
  }
}

