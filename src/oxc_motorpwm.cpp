#include <oxc_motorpwm.h>

using namespace oxc;

ReturnCode oxc::MotorPwm1P2D::set_v( float v )
{
  int dir = 0;
  if( v < -zero_v ) {
    v = -v; dir = -1;
  } else if( v > zero_v ) {
    dir = 1;
  } else {
    v = 0;
  }
  v = std::clamp( v, -1.0f, 1.0f );

  stop();
  if( !pwmc.setPwm( ch, v ) ) {
      return rcErr;
  }

  if( dir > 0 ) {
    pin_r.set();
  } else if( dir < 0 ) {
    pin_l.set();
  }

  return rcOk;
}

ReturnCode oxc::MotorPwm1P2D::set_x( float x )
{
  return rcErr;
}

ReturnCode oxc::MotorPwm1P2D::stop()
{
  pin_l.reset(); pin_r.reset();
  pwmc.setPwm( ch, 0 );
  return rcOk;
}

ReturnCode oxc::MotorPwm1P2D::brk()
{
  pin_l.set(); pin_r.set();
  pwmc.setPwm( ch, 0 );
  return rcOk;
}

ReturnCode oxc::MotorPwm1P2D::idle()
{
  return stop(); // param?
}


