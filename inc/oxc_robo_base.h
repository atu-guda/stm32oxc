#ifndef _OXC_ROBO_BASE_H
#define _OXC_ROBO_BASE_H

//* base definitions for robo parts in the oxc library

#include <oxc_types.h>

using std::size_t;
using std::int32_t;

namespace oxc {

//* base abstract class for interface to hardware devices
class RoboDevice {
  public:
   template<size_t N> // for only string literals as name
     constexpr explicit RoboDevice( const char (&name_)[N] ) noexcept : name( name_ ) {};
   RoboDevice( const RoboDevice &rhs ) = delete;
   virtual ~RoboDevice()  = default;
   const char* getName() const noexcept { return name; }
   virtual ReturnCode measure()   = 0;
   virtual ReturnCode commit()    = 0;
   virtual ReturnCode initHW()    = 0;
  protected:
   const char *name; // not own: only for debug
};



//* interfaces for actuators
class ActuPositionSink {
  public:
    virtual ReturnCode setQ( float q ) = 0;
};

class ActuVelocitySink
{
  public:
    virtual ReturnCode setV( float v ) = 0;
};

class ActuForceSink
{
  public:
    virtual ReturnCode setTau( float tau ) = 0;
};




}; // namespace oxc


#endif

