#include <oxc_actu_dcpwm.h>

using namespace oxc;

ReturnCode oxc::ActuDcPwm_1P2D::setX( float x )
{
  return rcErr;
}

ReturnCode oxc::ActuDcPwm_1P2D::setV( float v )
{
  int8_t dir { 0 };
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
    pin_r.accept( 0, 1 );
  } else if( dir < 0 ) {
    pin_l.accept( 0, 1 );
  }

  return rcOk;
}

ReturnCode oxc::ActuDcPwm_1P2D::setTo( float to )
{
  return rcErr;
}

ReturnCode oxc::ActuDcPwm_1P2D::stop()
{
  pin_l.accept( 0, 0 ); pin_r.accept( 0, 0 );
  pwmc.setPwm( ch, 0 );
  return rcOk;
}

ReturnCode oxc::ActuDcPwm_1P2D::brk()
{
  pin_l.accept( 0, 1 ); pin_r.accept( 0, 1 );
  pwmc.setPwm( ch, 0 );
  return rcOk;
}

ReturnCode oxc::ActuDcPwm_1P2D::idle()
{
  return stop(); // param?
}


