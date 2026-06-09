#ifndef _OXC_ACTU_SERVO_C_LWM_H
#define _OXC_ACTU_SERVO_C_LWM_H

#include <oxc_actu_servo_lwm_base.h>

namespace oxc {

// contigous LWM controlled servo
class ActuServoContLWM : public ActuServoLWMBase {
  public:
   ActuServoContLWM( PwmCtl &pwmc_, std::size_t ch_ )
     : ActuServoLWMBase( pwmc_, ch_ )
   {
     props = canSetV | canBreak;
   }
   virtual ReturnCode setX(  float x ) override { return rcErr; }
   virtual ReturnCode setV(  float v ) override { return setXV( v ); }
   virtual ReturnCode setTo( float v ) override { return rcErr;  }
   virtual ReturnCode stop() override { pwmc.setPwm( ch, 0 ); return rcOk; }
   virtual ReturnCode brk()  override { return rcOk; };
   virtual ReturnCode idle() override { return stop(); };
  protected:
};


}; // namespace oxc;

#endif

