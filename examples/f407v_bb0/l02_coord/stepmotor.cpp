#include "stepmotor.h"

void StepMotorGpio2::step()
{
  if( dir == 0 ) {
    return;
  }

  pins.set( pinStep );
  delay_mcs( pulse_us );
  pins.reset( pinStep );
}

void StepMotorGpio2::set_dir( int d )
{
  if( dir == d ) {
    return;
  }
  dir = d;

  if( dir >= 0 ) {
    pins.reset( pinDir );
  } else {
    pins.set(   pinDir );
  }
  delay_mcs( pulse_us );
}
