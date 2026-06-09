#ifndef _OXC_ACTU_SERVO_LWM_H
#define _OXC_ACTU_SERVO_LWM_H

#include <oxc_actu_servo_lwm_base.h>

namespace oxc {


// standard LWM controlled servo
class ActuServoLWM : public ActuServoLWMBase {
  public:
   ActuServoLWM( PwmCtl &pwmc_, std::size_t ch_ )
     : ActuServoLWMBase( pwmc_, ch_ )
   {
     props = canSetX | canBreak;
   }
   virtual ReturnCode setX(  float x  ) override { return setXV( x ); }
   virtual ReturnCode setV(  float v  ) override { return rcErr; }
   virtual ReturnCode setTo( float to ) override { return rcErr; }
   virtual ReturnCode stop() override { pwmc.setPwm( ch, 0 ); return rcOk; }
   virtual ReturnCode brk()  override { return rcOk; };
   virtual ReturnCode idle() override { return stop(); };

  protected:

};


}; // namespace oxc;

#endif

