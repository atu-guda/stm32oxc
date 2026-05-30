#include <oxc_motor_servo_lwm_base.h>

using namespace oxc;

ReturnCode oxc::MotorServoLWMBase::set_xv( float xv )
{
  xv = std::clamp( xv, -1.0f, 1.0f );
  auto pu = uint32_t( t_on_cen + t_on_dlt * xv );
  return pwmc.setPulse( ch, pu ) ? rcOk: rcErr;
}



