#ifndef _OXC_MOTORPWM_H
#define _OXC_MOTORPWM_H

#include <oxc_motor.h>
#include <oxc_gpio.h>

namespace oxc {


// PWM controlled: 1 pwm channel, 2 direction
class MotorPwm1P2D : public MotorBase {
  public:
   MotorPwm1P2D( PwmCtl &pwmc_, std::size_t ch_, PinBase &pin_l_, PinBase &pin_r_ )
     : pwmc( pwmc_ ), ch(ch_), pin_l( pin_l_ ), pin_r( pin_r_ )
   {
     props = canSetV | canBreak;
   }
   virtual ReturnCode set_v( float v ) override;
   virtual ReturnCode set_x( float x ) override;
   virtual ReturnCode stop() override;
   virtual ReturnCode brk() override;
   virtual ReturnCode idle() override;
   virtual float get_v() const override { return 0; }
   virtual float get_x() const override { return 0; }

   void set_zeto_v( float v_ ) { zero_v = v_; };
  protected:
   PwmCtl &pwmc;
   std::size_t ch;
   PinBase &pin_l;
   PinBase &pin_r;
   float  zero_v { 1e-6 };

};


}; // namespace oxc;

#endif

