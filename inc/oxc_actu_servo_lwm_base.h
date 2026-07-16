#ifndef _OXC_ACTU_SERVO_LWM_BASE_H
#define _OXC_ACTU_SERVO_LWM_BASE_H

#include <oxc_robopwmctl.h>

namespace oxc {


// base: LWM controlled servo: not Robo*, just common code
class ActuServoLWMBase {
  public:
   ActuServoLWMBase( RoboPwmCtl &pwmc_, std::size_t ch_ )
     : pwmc( pwmc_ ), ch(ch_)   {  }
   ReturnCode setXV( float xv ); // base: x or v [-1:1]

   void set_t_on_limits( uint32_t t_on_min_, uint32_t t_on_max_ )
   {
     t_on_min = t_on_min_;
     t_on_max = t_on_max_;
     t_on_cen = ( t_on_max + t_on_min ) / 2;
     t_on_dlt = ( t_on_max - t_on_min ) / 2; // /2 as x: +-1
   };
  protected:
   RoboPwmCtl &pwmc;
   std::size_t ch;
   uint32_t t_on_min {  500 }; // in us
   uint32_t t_on_max { 2500 };
   uint32_t t_on_cen { 1500 };
   uint32_t t_on_dlt { 1000 };

};


}; // namespace oxc;

#endif

