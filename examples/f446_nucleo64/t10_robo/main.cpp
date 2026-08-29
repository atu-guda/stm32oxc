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

using std::array;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test misc robo parts. TMP." NL;



// ------------------------ - local commands; ---------------------------------------
DCL_CMD_REG(      test_pin_d,   'T',      " [arg ] - pin1_d"  );
DCL_CMD_REG(      test_pins_hd, '\0',     " [arg ] - pins_hd"  );
DCL_CMD_REG(      wci,          '\0',     " i ch vi - write cap int"  );
DCL_CMD_REG(      wcf,          '\0',     " i ch vf - write cap float"  );
DCL_CMD_REG(      rci,          '\0',     " i ch - read cap int"  );
DCL_CMD_REG(      rcf,          '\0',     " i ch - read cap float"  );
DCL_CMD_REG(      wri,          '\0',     " i ch vi - write robo int"  );
DCL_CMD_REG(      wrf,          '\0',     " i ch vf - write robo float"  );
DCL_CMD_REG(      rri,          '\0',     " i ch - read robo int"  );
DCL_CMD_REG(      rrf,          '\0',     " i ch - read robo float"  );
DCL_CMD_REG(      measure,      'M',      " i - measure robo"  );
DCL_CMD_REG(      commit,       'C',      " i - commit robo"  );
DCL_CMD_REG(      list_ob,      'L',      " - list objects"  );
DCL_CMD_REG(      init,         '\0',     " - init"  );

// -------------------------------------------------------------------------------------

ReturnCode init_hw_all();

// ------------------------ Devices: capabilities ; ---------------------------------------

Gpio_Pin_Dev      pin1_hd( PC10 );
PinCapability     pin1_d(  pin1_hd );
PinRoboCapability pin1_rd( pin1_hd, 100 );

Gpio_Pin_Dev      pin2_hd( PC11 );
PinCapability     pin2_d(  pin1_hd );
PinRoboCapability pin2_rd( pin2_hd, 101 );

Gpio_Pin_Dev      pini_hd( PC13 );
PinCapability     pini_d(  pini_hd );
PinRoboCapability pini_rd( pini_hd, 102 );

Gpio_Pins_Dev      pins_hd( PC0, 4 ); // copy of leds
PinsCapability     pins_d(  pins_hd );
PinsRoboCapability pins_rd( pins_hd, 200 );


// ------------------------ - local sensors ; ---------------------------------------


// ------------------------ - local sensors end ---------------------------------------

// TestRoboDevice test_rd{ 112 };

Gpio_Pin_Dev* hw_pin[] {
  &pin1_hd,
  &pin2_hd,
  &pini_hd,
};

IoCapability* caps[] {
  &pin1_d,
  &pin2_d,
  &pini_d,
  &pins_d,
};

IoRoboCapability* rcaps[] {
  &pin1_rd,
  &pin2_rd,
  &pini_rd,
  &pins_rd,
};

RoboObject* robo_objs[] {
  &pin1_rd,
  &pin2_rd,
  &pini_rd,
  &pins_rd,
};


RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( robo_objs, robo_joints );


void idle_main_task()
{
  robo.at_main_idle();
}

void test_pinx_hd();
void test_pins_hd();

int main(void)
{
  BOARD_PROLOG;

  UVAR_a =    1; // auto commit
  UVAR_l =    1; // idle after run ?
  UVAR_n =   20; // n test
  UVAR_s = 1000; // scale
  UVAR_t =  100; // default delay

  if( ! init_hw_all().isOk() ) {
    std_out << "# Error: HW init" << NL;
    die4led( 1_mask );
  };

  BOARD_POST_INIT_BLINK;

  // oxc_add_aux_tick_fun( led_task_nortos ); // tmp disable to test leds

  robo.start_time();

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  pin1_hd.initHW();
  pin2_hd.initHW();
  // pini_hd.getPin()->dev().cfgIn();
  PC13.cfgIn();

  pins_hd.initHW(); // dup from leds, but may be another pins?

  pin2_rd.setFlags( RoboObject::noMeasure );
  pini_rd.setFlags( RoboObject::noCommit );

  return robo.init_all();
}


CMD_FUNCTION( test_pin_d )
{
  test_pinx_hd();
  return 0;
}

CMD_FUNCTION( test_pins_hd )
{
  test_pins_hd();
  return 0;
}


void out_pin_st( Gpio_Pin_Dev *ppin )
{
  std_out << '[' << ppin->read().value_or(5) << ' ' << ppin->readwr().value_or(6) << ']';
}

void test_pinx_hd()
{
  const uint32_t n = UVAR_n;
  for( uint32_t i=0; i<n; ++i ) {
    for( auto ppin : hw_pin ) {
      ppin->set(); out_pin_st( ppin );
    }
    delay_ms( UVAR_t );
    for( auto ppin : hw_pin ) {
      ppin->reset(); out_pin_st( ppin );
    }
    std_out << NL;
    delay_ms( UVAR_t );
  }

  for( uint32_t i=0; i<n; ++i ) {
    for( auto ppin : hw_pin ) {
      ppin->toggle(); out_pin_st( ppin );
    }
    std_out << NL;
    delay_ms( UVAR_t );
  }

  for( uint32_t i=0; i<n; ++i ) {
    for( auto ppin : hw_pin ) {
      ppin->write( i ); out_pin_st( ppin );
    }
    delay_ms( UVAR_t );
    std_out << NL;
  }

}


void out_pins_st( Gpio_Pins_Dev *ppins )
{
  std_out << '[' << ppins->read().value_or(7) << ' ' << ppins->readwr().value_or(8) << ']';
}

void test_pins_hd()
{
  const uint32_t n = UVAR_n;

  std_out << "# write" NL;
  for( uint32_t i=0; i<n; ++i ) {
    std_out << i << ' ';
    pins_hd.write( i ); out_pins_st( &pins_hd );
    std_out << NL;
    delay_ms( UVAR_t );
  }

  std_out << "# set/reset" NL;
  pins_hd.reset( 0xFF );
  for( uint32_t i=0; i<n; ++i ) {
    std_out << i << ' ';
    pins_hd.set( i ); out_pins_st( &pins_hd );
    delay_ms( UVAR_t );
    pins_hd.set( 0xFF );
    pins_hd.reset( i ); out_pins_st( &pins_hd );
    std_out << NL;
  }

  std_out << "# toggle" NL;
  pins_hd.reset( 0xFF );
  for( uint32_t i=0; i<n; ++i ) {
    std_out << i << ' ';
    pins_hd.toggle( i ); out_pins_st( &pins_hd );
    delay_ms( UVAR_t );
    std_out << NL;
  }


}

CMD_FUNCTION( wci )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(caps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  int32_t vi  = arg2long_d(  3, argc, argv, 0 );
  auto rc = caps[idx]->setVal( ch, vi );
  std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  return 0;
}


CMD_FUNCTION( wcf )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(caps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  float vf = arg2float_d( 3, argc, argv, 0 );
  auto rc = caps[idx]->setValF( ch, vf );
  std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  return 0;
}


CMD_FUNCTION( rci )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(caps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  auto r = caps[idx]->getVal( ch );
  if( r ) {
    std_out << "# val= " << r.value() << NL;
  } else {
    std_out << "# eer " << r.error().code << NL;
  }
  return 0;
}

CMD_FUNCTION( rcf )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(caps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  auto r = caps[idx]->getValF( ch );
  if( r ) {
    std_out << "# val= " << r.value() << NL;
  } else {
    std_out << "# eer " << r.error().code << NL;
  }
  return 0;
}



CMD_FUNCTION( wri )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(rcaps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  int32_t vi  = arg2long_d(  3, argc, argv, 0 );
  auto rc = rcaps[idx]->setVal( ch, vi );
  std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  if( UVAR_a ) {
    rc = rcaps[idx]->commit();
    std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  }
  return 0;
}


CMD_FUNCTION( wrf )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(rcaps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  float vf = arg2float_d( 3, argc, argv, 0 );
  auto rc = rcaps[idx]->setValF( ch, vf );
  std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  if( UVAR_a ) {
    rc = rcaps[idx]->commit();
    std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  }
  return 0;
}


CMD_FUNCTION( rri )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(rcaps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  auto r = rcaps[idx]->getVal( ch );
  if( r ) {
    std_out << "# val= " << r.value() << NL;
  } else {
    std_out << "# eer " << r.error().code << NL;
  }
  return 0;
}

CMD_FUNCTION( rrf )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(rcaps)-1 );
  auto ch  = arg2ulong_d( 2, argc, argv, 0 );
  auto r = rcaps[idx]->getValF( ch );
  if( r ) {
    std_out << "# val= " << r.value() << NL;
  } else {
    std_out << "# eer " << r.error().code << NL;
  }
  return 0;
}

CMD_FUNCTION( measure )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(rcaps)-1 );
  auto rc = rcaps[idx]->measure();
  std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  return 0;
}

CMD_FUNCTION( commit )
{
  auto idx = arg2ulong_d( 1, argc, argv, 0, 0, std::size(rcaps)-1 );
  auto rc = rcaps[idx]->commit();
  std_out << "# rc= " << rc.code << ' ' << rc.data << NL;
  return 0;
}

CMD_FUNCTION( list_ob )
{
  for( auto [i,pob] : std::views::enumerate( robo_objs ) ) {
    std_out << i << ' ' << pob->getId() << ' ' << pob->getStatus() << ' ' << pob->getDirty()
            << ' ' << pob->getFlags() << ' ' << HexInt(pob) << NL;
  }
  return 0;
}

CMD_FUNCTION( init )
{
  auto rc = robo.init_all();
  std_out << " rc= [" << rc.code << ", " << rc.data << "]" NL;
  return 0;
}


