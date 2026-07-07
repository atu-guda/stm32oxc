#ifndef _OXC_ACTU_BASE_H
#define _OXC_ACTU_BASE_H

// only for ReturnCode
#include <oxc_types.h>
#include <oxc_coordtransform.h>

using std::size_t;

namespace oxc {


class Actuator { // obsoleted: use Robo*Sync
  public:
   enum Props {
     canSetX  =     1, canSetV = 2, canSetTorque = 4,
     canBreak =  0x10
   };
   enum Flags {
     flagReady = 1, flagError = 2, flagIdle = 4
   };
   // TODO non-virt setX + virtual do_setX...
   virtual ReturnCode setX(  float x )  = 0;
   virtual ReturnCode setV(  float v )  = 0;
   virtual ReturnCode setTo( float to ) = 0;
   virtual ReturnCode stop() = 0;
   virtual ReturnCode brk()  = 0;
   virtual ReturnCode idle() = 0;
   virtual ReturnCode init() = 0;
   uint32_t getProps() const { return props; }
   uint32_t getFlags() const { return flags; }
  protected:
   // CoordTransform &coo_tr;
   uint32_t props {0};
   uint32_t flags {0};
};



}; // namespace oxc;

#endif

