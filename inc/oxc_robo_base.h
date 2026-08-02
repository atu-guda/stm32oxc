#ifndef _OXC_ROBO_BASE_H
#define _OXC_ROBO_BASE_H

//* base definitions for robo parts in the oxc library

#include <span>

#include <oxc_coordtransform.h>
#include <oxc_capabilities.h>


namespace oxc {

//* base abstract class for interface to hardware devices
class RoboDevice {
  public:
   constexpr explicit RoboDevice( uint32_t id_ ) noexcept : id( id_ ) {};
   RoboDevice( const RoboDevice &rhs ) = delete;
   virtual ~RoboDevice()  = default;
   uint32_t getId() const noexcept { return id; }
   ReturnCode status() const noexcept   { return sta; }
   virtual ReturnCode measure()   = 0;
   virtual ReturnCode commit()    = 0;
   virtual ReturnCode initHW()    = 0;
  protected:
   ReturnCode sta { ReturnCode::rcnErr, 1 }; // uninitialised
   uint32_t id; //* simple id for debug
};

//* just to test design - second try
class TestRoboDevice : public IoRoboCapability {
  public:
   enum { xx_n_ch = 4, xx_bitsz = 32 };
   TestRoboDevice() : IoRoboCapability( xx_n_ch, xx_bitsz, xx_buf ) {};
  protected:
   virtual ReturnCode doInit()    noexcept override { std::ranges::fill( xx_buf, 0 ); dirty = false; return rcOk; };
   virtual ReturnCode doMeasure() noexcept override { return rcOk; };
   virtual ReturnCode doThink()   noexcept override { return rcOk; };
   virtual ReturnCode doCommit()  noexcept override { xx_t = xx_buf[0]; dirty = false; return rcOk; };
   int32_t xx_buf[xx_n_ch];
   int32_t xx_t {0};
};



//* fake Robo device - for test purpose
class FakeRoboDevice : public RoboDevice {
  public:
   constexpr explicit FakeRoboDevice( uint32_t id_ ) noexcept : RoboDevice( id_ ) {};
   virtual ReturnCode measure() override { return rcOk; }
   virtual ReturnCode commit()  override { return rcOk; }
   virtual ReturnCode initHW()  override { return rcOk; }
};

//* physycal part of robo sensors with channels
class RoboSensor : public RoboDevice {
  public:
   constexpr explicit RoboSensor( uint32_t id_, size_t n_ch_ ) noexcept
     : RoboDevice( id_ ), n_ch ( n_ch_ ) {};
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


//* interfaces for actuators // TODO: capabilities? 
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
   ReturnCode init_all();
   ReturnCode measure_all();
   ReturnCode commit_all();
   RoboDevice* get_last_err_dev() const { return last_err_dev; }
   void set_measure_idle_ticks( uint32_t v ) { measure_idle_ticks = v; }

   void start_time();
   void calc_current_time();
   void at_main_idle();
   uint32_t get_t_cur_i() const { return t_cur_i; }
   float    get_t_cur()   const { return t_cur_f; }
   uint32_t get_t_dt_i()  const { return t_dt;    }
   float    get_t_dt()    const { return t_dt_f;  }

  protected:
   std::span<RoboDevice*>      pdevs;
   std::span<RoboJoint*>      joints;
   RoboDevice* last_err_dev { nullptr };
   uint32_t t_start_i       {       0 };
   uint32_t t_cur_i         {       0 };
   uint32_t t_dt            {       0 };
   float    t_cur_f         {    0.0f };
   float    t_dt_f          {    0.0f };
   uint32_t t_meas_i        {       0 };
   uint32_t measure_idle_ticks {  100 };
   bool     first_measure   {    true };

};



}; // namespace oxc


#endif

