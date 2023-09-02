#ifndef _MAIN_H
#define _MAIN_H

#include <oxc_gcode.h>

const inline uint32_t  TIM_PWM_base_freq   { 84'000'000 };
const inline uint32_t  TIM_PWM_count_freq  {      10000 };
const inline uint32_t  TIM6_base_freq   {  1'000'000 };
const inline uint32_t  TIM6_count_freq  {      20000 };

extern int debug; // in main.cpp

// mach params
struct MachParam {
  uint32_t tick2mm   ; // tick per mm, = 2* pulses per mm
  uint32_t max_speed ; // mm/min
  uint32_t max_l     ; // mm
  float    es_find_l ; // movement to find endstop, from ES, to = *1.5
  float    k_slow    ; // slow movement coeff from max_speed
  PinsIn  *endstops  ;
  PinsOut *motor     ;
};

const inline constinit unsigned n_motors { 5 };

extern MachParam machs[n_motors];

int gcode_cmdline_handler( char *s );

class MachState : public MachStateBase {
  public:
   MachState( fun_gcode_mg prep, const FunGcodePair *g_f, const FunGcodePair *m_f );
   enum MachMode {
     modeFFF = 0, modeLaser = 1, modeCNC = 2, modeMax = 3
   };
   // TODO: protected:
   MachMode mode { modeFFF };
   xfloat x[n_motors];
   xfloat axis_scale[n_motors];
   xfloat fe_g0 { 350 };
   xfloat fe_g1 { 300 };
   xfloat fe_scale { 100.0f };
   xfloat spin  {   0 };
   xfloat spin100  { 10000 }; // scale for laser PWM
   xfloat spin_max {  90 };   // max PWM in %
   uint32_t n_mo { 0 }; // current number of active motors
   uint32_t last_rc;
   bool was_set { false };
   bool relmove { false };
   bool inchUnit { false };
   bool spinOn   { false };
   xfloat getPwm() const { return std::clamp( 100 * spin / spin100, 0.0f, spin_max ); }
};

extern MachState me_st;
int mach_prep_fun( GcodeBlock *cb, MachStateBase *ms );
extern const MachStateBase::FunGcodePair mach_g_funcs[];
extern const MachStateBase::FunGcodePair mach_m_funcs[];

// move task description
struct MoveTask1 {
  int8_t   dir;   // 1-forvard, 0-no, -1 - backward
  int step_rest;  // downcount of next
  int step_task;  // total ticks in this task
  int d;          // for Brese
  inline void init() { dir = 0; step_rest = step_task = d = 0; }
};

extern MoveTask1 move_task[n_motors+1]; // last idx = time

void motors_off();
void motors_on();
int pwm_set( unsigned idx, float v );
int pwm_off( unsigned idx );
int pwm_off_all();
int move_rel( const float *d_mm, unsigned n_mo, float fe_mmm );
int go_home( unsigned axis );

inline bool is_endstop_minus_stop( uint16_t e ) { return ( (e & 0x01) == 0 ) ; }
inline bool is_endstop_minus_go(   uint16_t e ) { return ( (e & 0x01) != 0 ) ; }
inline bool is_endstop_plus_stop(  uint16_t e ) { return ( (e & 0x02) == 0 ) ; }
inline bool is_endstop_plus_go(    uint16_t e ) { return ( (e & 0x02) != 0 ) ; }
inline bool is_endstop_any_stop(   uint16_t e ) { return ( (e & 0x03) != 3 ) ; }
inline bool is_endstop_clear(      uint16_t e ) { return ( (e & 0x03) == 3 ) ; }
inline bool is_endstop_bad(        uint16_t e ) { return ( (e & 0x03) == 0 ) ; }

#endif

