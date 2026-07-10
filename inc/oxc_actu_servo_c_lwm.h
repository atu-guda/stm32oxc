#ifndef _OXC_ACTU_SERVO_C_LWM_H
#define _OXC_ACTU_SERVO_C_LWM_H

#include <oxc_actu_servo_lwm_base.h>
#include <oxc_coordtransform.h>

namespace oxc {


// contigous LWM controlled servo
class ActuServoContLWM : public ActuServoLWMBase, public ActuVelocitySink {
  public:
   ActuServoContLWM( RoboPwmCtl &pwmc_, std::size_t ch_, CoordTransform &coo_tr_ )
     : ActuServoLWMBase( pwmc_, ch_ ), coo_tr( coo_tr_ ) {}
   virtual ReturnCode setV( float v ) override {
     v_int = std::clamp( coo_tr.toInternal( v ), -1.0f, 1.0f );
     dir = 1; /// ????
     v_phy = coo_tr.toPhys( v_int );
     return setXV( v_int );
   }
   virtual ReturnCode brk()  override { return pwmc.setPulse( ch, t_on_cen ); }
   virtual ReturnCode idle() override { return pwmc.setPulse( ch, 0 ); }
  protected:
   CoordTransform &coo_tr;

};


}; // namespace oxc;

#endif

