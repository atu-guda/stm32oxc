#include "stepmover.h"


StepMover::StepMover( StepMotor &a_motor, EndStop *a_endstops, uint32_t a_tick_2mm, uint32_t a_max_speed, uint32_t a_max_l )
  : motor( a_motor ), endstops( a_endstops ), tick2mm( a_tick_2mm ), max_speed( a_max_speed ), max_l( a_max_l )
{
}

void StepMover::initHW()
{
  if( endstops ) {
    endstops->initHW();
  }
  motor.initHW();
};

void StepMover::set_dir( int a_dir )
{
  motor.set_dir( a_dir );
}

ReturnCode StepMover::step()
{
  auto dir = motor.get_dir();
  if( dir == 0 ) {
    return rcOk;
  }

  auto rc = check_es();
  if( rc != rcOk ) {
    return rc;
  }

  if( true_mode ) {
    motor.step();
  }
  x += dir;
  return rcOk;
}

ReturnCode StepMover::step_to( xfloat to )
{
  int to_i = mm2tick( to );
  int d = ( to_i > x ) ? 1 : ( ( to_i < x ) ? -1 : 0 );
  return step_dir( d );
}

ReturnCode StepMover::check_es()
{
  auto dir = motor.get_dir();
  if( ! endstops || dir == 0 ) {
    return rcOk;
  }
  endstops->read();
  if( endstops->is_bad() ) {
    return rcErr;
  }

  switch( es_mode ) {
    case EndstopMode::All:
      return endstops->is_clear() ? rcOk : rcEnd;
    case EndstopMode::Dir:
      return endstops->is_clear_for_dir( dir ) ? rcOk : rcEnd;
    case EndstopMode::From: // move from endstop
      return ( endstops->is_clear_for_dir( dir ) && ! endstops->is_clear() ) ? rcOk : rcEnd;
  }
  return rcErr; // unlikely
}

