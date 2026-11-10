#ifndef _OXC_ROBO_BASE_H
#define _OXC_ROBO_BASE_H

//* base definitions for robo parts in the oxc library

#include <span>

// for DCL_*
#include <oxc_console.h>
#include <oxc_robocaps.h>


namespace oxc {



struct ActuatorLimits
{
  float max_v;
  float max_a;
  float max_eff;
};



// base for the Controller
// class RoboJointCtl {
// };




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

extern RoboAssembly& global_robo;


}; // namespace oxc

// definitions - in oxc_robo_debug.cpp
DCL_CMD_ONLY(      robo_wi   );
DCL_CMD_ONLY(      robo_wf   );
DCL_CMD_ONLY(      robo_ri   );
DCL_CMD_ONLY(      robo_rf   );
DCL_CMD_ONLY(      robo_meas );
DCL_CMD_ONLY(      robo_co   );
DCL_CMD_ONLY(      robo_list );
DCL_CMD_ONLY(      robo_init );
DCL_CMD_ONLY(      out_cf    );
DCL_CMD_ONLY(      out_ci    );
DCL_CMD_ONLY(      in_cf     );
DCL_CMD_ONLY(      in_ci     );


#endif

