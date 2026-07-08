#ifndef _OXC_ACTU_SERVO_LWM_H
#define _OXC_ACTU_SERVO_LWM_H

#include <oxc_actu_servo_lwm_base.h>
#include <oxc_coordtransform.h>

namespace oxc {


// standard LWM controlled servo
class ActuServoLWM : public ActuServoLWMBase, public ActuPositionSink {
  public:
   ActuServoLWM( RoboPwmCtl &pwmc_, std::size_t ch_, CoordTransform &coo_tr_ )
     : ActuServoLWMBase( pwmc_, ch_ ), coo_tr( coo_tr_ ) {}
   virtual ReturnCode setQ( float q ) override {
     q_int = std::clamp( coo_tr.toInternal( q ), -1.0f, 1.0f );
     q_phy = coo_tr.toPhys( q_int );
     return setXV( q_int );
   };
   virtual ReturnCode brk()           override { return rcOk; };
   virtual ReturnCode idle()          override { return pwmc.setPulse( ch, 0 ); };
  protected:
   CoordTransform &coo_tr;

};


}; // namespace oxc;

#endif

