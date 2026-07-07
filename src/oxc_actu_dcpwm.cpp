#include <oxc_actu_dcpwm.h>

using namespace oxc;


ReturnCode oxc::ActuDcPwm_1P2D::setV( float v_p )
{
  v_phy = v_p;
  v_int = coo_tr.toInternal( v_phy );

  dir = 0;
  if( v_int < -zero_v ) {
    v_int = -v_int; dir = -1;
  } else if( v_int > zero_v ) {
    dir = 1;
  } else {
    v_int = 0;
  }
  v_int = std::clamp( v_int, 0.0f, 1.0f );

  pin_l.set(); pin_r.set();
  if( !pwmc.setPwm( ch, v_int ) ) {
    return rcErr;
  }

  if( dir > 0 ) {
    pin_l.reset();
  } else if( dir < 0 ) {
    pin_r.reset();
  }

  return rcOk;
}


ReturnCode oxc::ActuDcPwm_1P2D::idle()
{
  pin_l.reset(); pin_r.reset();
  pwmc.setPwm( ch, 0 );
  v_phy = v_int = 0; dir = 0;
  return rcOk;
}

ReturnCode oxc::ActuDcPwm_1P2D::brk()
{
  pin_l.set(); pin_r.set();
  pwmc.setPwm( ch, 0 );
  v_phy = v_int = 0; dir = 0;
  return rcOk;
}



