#ifndef _OXC_MOTOR_SERVO_C_LWM_H
#define _OXC_MOTOR_SERVO_C_LWM_H

#include <oxc_motor.h>
#include <oxc_gpio.h>
#include <oxc_motor_servo_lwm_base.h>

namespace oxc {


// contigous LWM controlled servo
class MotorServoContLWM : public MotorServoLWMBase {
  public:
   MotorServoContLWM( PwmCtl &pwmc_, std::size_t ch_ )
     : MotorServoLWMBase( pwmc_, ch_ )
   {
     props = canSetV | canBreak;
   }
   virtual ReturnCode set_v( float v ) override { return set_xv( v ); }
   virtual ReturnCode set_x( float x ) override { return rcErr; }
   virtual ReturnCode stop() override { pwmc.setPwm( ch, 0 ); return rcOk; }
   virtual ReturnCode brk()  override { return rcOk; };
   virtual ReturnCode idle() override { return stop(); };
   // virtual float get_v() const override { return 0; }
   // virtual float get_x() const override { return 0; }
  protected:
};


}; // namespace oxc;

#endif

