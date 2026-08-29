#ifndef _OXC_ROBO_BASE_H
#define _OXC_ROBO_BASE_H

//* base definitions for robo parts in the oxc library

#include <span>

#include <oxc_robocaps.h>


namespace oxc {



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
   constexpr RoboAssembly( std::span<RoboObject*>  pdevs_,
                           std::span<RoboJoint*> joints_ ) noexcept
     : pdevs( pdevs_ ), joints( joints_ ) {}
   RoboAssembly( const RoboAssembly &rhs ) = delete;
   ReturnCode for_all_till_err( ReturnCode (RoboObject::*fun)() ) noexcept;
   ReturnCode init_all() noexcept;
   ReturnCode measure_all() noexcept;
   ReturnCode think_all() noexcept;
   ReturnCode commit_all() noexcept;
   RoboObject* get_last_err_dev() const  noexcept{ return last_err_dev; }
   void set_measure_idle_ticks( uint32_t v ) noexcept { measure_idle_ticks = v; }

   void start_time();
   void calc_current_time();
   void at_main_idle();
   uint32_t get_t_cur_i() const noexcept { return t_cur_i; }
   float    get_t_cur()   const noexcept { return t_cur_f; }
   uint32_t get_t_dt_i()  const noexcept { return t_dt;    }
   float    get_t_dt()    const noexcept { return t_dt_f;  }

  protected:
   std::span<RoboObject*>  pdevs;
   std::span<RoboJoint*>        joints;
   RoboObject* last_err_dev { nullptr };
   uint32_t t_start_i       {       0 }; // TODO: combine to structure
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

