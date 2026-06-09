#ifndef _OXC_ACTU_SERVO_LWM_BASE_H
#define _OXC_ACTU_SERVO_LWM_BASE_H

#include <oxc_actu_base.h>
#include <oxc_pwmctl.h>

namespace oxc {


// base: LWM controlled servo
class ActuServoLWMBase : public Actuator {
  public:
   ActuServoLWMBase( PwmCtl &pwmc_, std::size_t ch_ )
     : pwmc( pwmc_ ), ch(ch_)   {  }
   ReturnCode setXV( float xv ); // base: x or v
   // all other unimplemented
   // virtual ReturnCode set_v( float v ) override;
   // virtual ReturnCode set_x( float x ) override;
   // virtual ReturnCode stop() override;
   // virtual ReturnCode brk() override;
   // virtual ReturnCode idle() override;
   virtual ReturnCode init() { return rcOk; /* pwmc.initHW() ? rcOk : rcErr; */ } // TODO: fix

   void set_t_on_limits( uint32_t t_on_min_, uint32_t t_on_max_ )
   {
     t_on_min = t_on_min_;
     t_on_max = t_on_max_;
     t_on_cen = ( t_on_max + t_on_min ) / 2;
     t_on_dlt = ( t_on_max - t_on_min ) / 2; // /2 as x: +-1
   };
  protected:
   PwmCtl &pwmc;
   std::size_t ch;
   uint32_t t_on_min {  500 }; // in us
   uint32_t t_on_max { 2500 };
   uint32_t t_on_cen { 1500 };
   uint32_t t_on_dlt { 1000 };

};


}; // namespace oxc;

#endif

