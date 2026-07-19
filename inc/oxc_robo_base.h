#ifndef _OXC_ROBO_BASE_H
#define _OXC_ROBO_BASE_H

//* base definitions for robo parts in the oxc library

#include <span>

#include <oxc_coordtransform.h>

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
   ReturnCode status() const noexcept   { return sta; }
   virtual ReturnCode measure()   = 0;
   virtual ReturnCode commit()    = 0;
   virtual ReturnCode initHW()    = 0;
  protected:
   ReturnCode sta { ReturnCode::rcnErr, 1 }; // uninitialised
   const char *name; // not own: only for debug
};

//* physycal part of robo sensors with channels
class RoboSensor : public RoboDevice {
  public:
   template<size_t N> // for only string literals as name
     constexpr explicit RoboSensor( const char (&name_)[N], size_t n_ch_ ) noexcept
     : RoboDevice( name_ ), n_ch ( n_ch_ ) {};
   virtual ReturnCode commit() override { return rcOk; }
   virtual int32_t get( size_t ch ) = 0; // single-channel sensors may ignore ch
   virtual int32_t getScale( size_t ch ) = 0; // single-channel sensors may ignore ch
   virtual void setVal( size_t ch, int32_t v ) {}; // by default - do nothing
   size_t size() const { return n_ch; };

  protected:
   const size_t n_ch;
};

//* logical sensor: selects and scale 1 channel of the RoboSensor
class SensorBase {
  public:
   explicit constexpr SensorBase( RoboSensor &psens_, size_t ch_, CoordTransform &coo_tr_ )
     : psens( psens_ ), ch( ch_ ), coo_tr( coo_tr_ ) {}
   virtual ~SensorBase() = default;
   virtual float     get()  { return coo_tr.toPhys( psens.get( ch ) ); }
   virtual int32_t get_i()  { return                psens.get( ch ); }
  protected:
   RoboSensor &psens;
   const size_t ch;
   CoordTransform &coo_tr;
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

// base for the Controller
class RoboJointCtl {
};

//* simple "set posision" controller, like for LWM Servo
class RoboJointCtlPos : public RoboJointCtl {
};



class RoboJoint {
  // public:
  //  RoboJoint( SensorBase &se_, Actuator &actu_, RoboJointCtl &ctl )
  //    : se( se_ ), actu( actu_ ), ctl( ctl ) {}
  //
  //  float get() const { return se.get(); }
  //
  // protected:
  //  SensorBase &se;
  //  Actuator &actu;
  //  RoboJointCtl &ctl;
};


class RoboAssembly {
  public:
   constexpr RoboAssembly( std::span<RoboDevice*>  pdevs_,
                           std::span<RoboJoint*> joints_ ) noexcept
     : pdevs( pdevs_ ), joints( joints_ ) {}
   RoboAssembly( const RoboAssembly &rhs ) = delete;
   ReturnCode for_all_till_err( ReturnCode (RoboDevice::*fun)() );
   ReturnCode init_all()    { return for_all_till_err( &RoboDevice::initHW  ); }
   ReturnCode measure_all() { return for_all_till_err( &RoboDevice::measure ); };
   ReturnCode commit_all()  { return for_all_till_err( &RoboDevice::commit  ); }
  protected:
   std::span<RoboDevice*>      pdevs;
   std::span<RoboJoint*>      joints;

};



}; // namespace oxc


#endif

