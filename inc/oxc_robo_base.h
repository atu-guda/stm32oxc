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


struct ActuatorLimits
{
  float max_v;
  float max_a;
  float max_eff;
};


//* interfaces for actuators
class ActuPositionSink {
  public:
   virtual ReturnCode setQ( float q ) = 0;
   virtual ReturnCode brk()           = 0;
   virtual ReturnCode idle()          = 0;
   float get_q_phy() const { return q_phy; }
   float get_q_int() const { return q_int; }
  protected:
   float q_phy { 0 }; // physical
   float q_int { 0 }; // internal
};

class ActuVelocitySink
{
  public:
   virtual ReturnCode setV( float v_p ) = 0;
   virtual ReturnCode brk()             = 0;
   virtual ReturnCode idle()            = 0;
   float get_v_phy() const { return v_phy; }
   float get_v_int() const { return v_int * dir; }
  protected:
   float v_phy { 0 }; // physical
   float v_int { 0 }; // internal
   int8_t dir  { 0 };
};

class ActuForceSink
{
  public:
   virtual ReturnCode setTau( float tau ) = 0;
   float get_tau_phy() const { return tau_phy; }
   float get_tau_int() const { return tau_int; }
  protected:
   float tau_phy { 0 }; // physical
   float tau_int { 0 }; // internal
};




}; // namespace oxc


#endif

