#ifndef _SETMOVER_H
#define _SETMOVER_H

#include <oxc_base.h>
#include <oxc_floatfun.h>

#include "endstop.h"
#include "stepmotor.h"

// TODO: base: common props, here - realization
// TODO: ctor, [ptr]
// mach params
class StepMover {
  public:
   enum class EndstopMode { All, Dir, From };
   StepMover( StepMotor &a_motor, EndStop *a_endstops, uint32_t a_tick_2mm, uint32_t a_max_speed, uint32_t a_max_l );
   void initHW();
   void set_dir( int dir );
   ReturnCode step();
   ReturnCode step_dir( int dir ) { set_dir( dir ); return step(); }
   ReturnCode step_to( xfloat to );
   ReturnCode check_es();
   int  get_x() const { return x; }
   int  get_dir() const { return motor.get_dir(); }
   void set_x( int a_x ) { x = a_x; }
   xfloat  get_xf() const { return (xfloat)x / tick2mm; }
   void set_xf( xfloat a_x ) { x = a_x * tick2mm; }
   int  mm2tick( xfloat mm ) { return mm * tick2mm; }
   uint32_t get_max_speed() const { return max_speed; };
   uint32_t get_max_l() const { return max_l; };
   xfloat get_es_find_l() const { return es_find_l; }; // remove?
   xfloat get_k_slow() const { return k_slow; };
   StepMotor& get_motor() { return motor; } // not const, remove?
   EndStop* get_endstops() { return endstops; } // not const, remove? + ask funcs
   void set_es_mode( EndstopMode a_m ) { es_mode = a_m; }
   void set_true_mode( bool m ) { true_mode = m; }
   bool get_true_mode() const { return true_mode; }
  protected:
   StepMotor &motor     ;
   EndStop  *endstops ;
   EndstopMode es_mode { EndstopMode::Dir };
   uint32_t tick2mm   ; // tick per mm, =  pulses per mm
   uint32_t max_speed ; // mm/min
   uint32_t max_l     ; // mm
   int      x         { 0 }; // current pos in pulses, ? int64_t?
   xfloat   es_find_l { 5.0f }; // movement to find endstop, from=ES, to = *1.5
   xfloat   k_slow    { 0.1f }; // slow movement coeff from max_speed
   bool     true_mode { true };
};



#endif

