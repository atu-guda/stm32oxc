#ifndef _OXC_ACTU_DCPWM_H
#define _OXC_ACTU_DCPWM_H

#include <oxc_actu_base.h>
#include <oxc_robopin.h>
#include <oxc_robopwmctl.h>
#include <oxc_coordtransform.h>

namespace oxc {


// PWM controlled: 1 pwm channel, 2 direction
class ActuDcPwm_1P2D : public ActuVelocitySink {
  public:
   ActuDcPwm_1P2D( RoboPwmCtl &pwmc_, std::size_t ch_, RoboPin &pin_l_, RoboPin &pin_r_, CoordTransform &coo_tr_ )
     : pwmc( pwmc_ ), ch(ch_), pin_l( pin_l_ ), pin_r( pin_r_ ), coo_tr( coo_tr_ )
   { }
   virtual ReturnCode setV( float v ) override;
   virtual ReturnCode brk()  override;
   virtual ReturnCode idle() override;

   void set_zero_v( float v_ ) { zero_v = v_; };
  protected:
   RoboPwmCtl &pwmc;   //* any PWM controller with RoboDevice interface
   std::size_t ch;     //* channel in this controller - often used
   RoboPin &pin_l;     //* pins to set direction/stop mode
   RoboPin &pin_r;
   CoordTransform &coo_tr;
   float  zero_v { 1e-6 }; // if v is set lower then this - stop

};


}; // namespace oxc;

#endif

