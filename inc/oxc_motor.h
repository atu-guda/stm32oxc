#ifndef _OXC_MOTOR_H
#define _OXC_MOTOR_H

#include <oxc_base.h>
#include <oxc_pwmctl.h>

namespace oxc {

class MotorBase {
  public:
   enum Props {
     canSetV = 1, canSetX = 2, canBreak = 4, canGetV = 8, canGetX = 0x10
   };
   virtual ReturnCode set_v( float v ) = 0; // [-1;1]
   virtual ReturnCode set_x( float x ) = 0; // [-1;1]
   virtual ReturnCode stop() = 0;
   virtual ReturnCode brk() = 0;
   virtual ReturnCode idle() = 0;
   virtual float get_v() const = 0;
   virtual float get_x() const = 0;
   uint32_t getProps() const { return props; }
   uint32_t getFlags() const { return flags; }
  protected:
   uint32_t props {0};
   uint32_t flags {0};
};



}; // namespace oxc;

#endif

