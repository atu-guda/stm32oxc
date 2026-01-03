#include "stepmotor.h"

void StepMotorGpio2::step()
{
  if( dir == 0 ) {
    return;
  }

  pins.set(   PinMask( pinStep ) );
  delay_mcs( pulse_us );
  pins.reset( PinMask( pinStep ) );
}

void StepMotorGpio2::set_dir( int d )
{
  if( dir == d ) {
    return;
  }
  dir = d;

  if( dir >= 0 ) {
    pins.reset( PinMask( pinDir ) );
  } else {
    pins.set(   PinMask( pinDir ) );
  }
  delay_mcs( pulse_us );
}
