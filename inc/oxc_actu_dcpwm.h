#ifndef _OXC_ACTU_DCPWM_H
#define _OXC_ACTU_DCPWM_H

#include <oxc_actu_base.h>
#include <oxc_pwmctl.h>
#include <oxc_pinbase.h>

namespace oxc {


// PWM controlled: 1 pwm channel, 2 direction
class ActuDcPwm_1P2D : public Actuator {
  public:
   ActuDcPwm_1P2D( PwmCtl &pwmc_, std::size_t ch_, PinBase &pin_l_, PinBase &pin_r_ )
     : pwmc( pwmc_ ), ch(ch_), pin_l( pin_l_ ), pin_r( pin_r_ )
   {
     props = canSetV | canBreak;
   }
   virtual ReturnCode setX(  float x  ) override; // No
   virtual ReturnCode setV(  float v  ) override;
   virtual ReturnCode setTo( float to ) override; // No
   virtual ReturnCode stop() override;
   virtual ReturnCode brk()  override;
   virtual ReturnCode idle() override;
   virtual ReturnCode init() override { return initHW(); };

   ReturnCode initHW() {
     oxc::ReturnCode rc = pin_l.initHW();
     // pwmc.initHW() && 
     if( !rc ) return rc;
     rc = pin_r.initHW();
     return rc;
   }
   void set_zero_v( float v_ ) { zero_v = v_; };
  protected:
   PwmCtl &pwmc;   //* any PWM controller
   std::size_t ch; //* channel in this controller - often used
   PinBase &pin_l; //* pins to set direction/stop mode
   PinBase &pin_r;
   float  zero_v { 1e-6 }; // if v is set lower then this - stop

};


}; // namespace oxc;

#endif

