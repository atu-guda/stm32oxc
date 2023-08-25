#ifndef _MAIN_H
#define _MAIN_H

#include <oxc_gcode.h>

const inline uint32_t  TIM_PWM_base_freq   { 84'000'000 };
const inline uint32_t  TIM_PWM_count_freq  {      10000 };
const inline uint32_t  TIM6_base_freq   {  1'000'000 };
const inline uint32_t  TIM6_count_freq  {      20000 };

// mach params
struct MachParam {
  uint32_t tick2mm   ; // tick per mm, = 2* pulses per mm
  uint32_t max_speed ; // mm/min
  uint32_t max_l     ; // mm
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
   xfloat fe_g0 { 500 };
   xfloat fe_g1 { 300 };
   xfloat spin  {   0 };
   xfloat spin100  { 1000 };
   uint32_t n_mo { 0 }; // current number of active motors
   uint32_t last_rc;
   bool was_set { false };
   bool relmove { false };
   bool inchUnit { false };
   bool spinOn   { false };
};

extern MachState me_st;

// move task description
struct MoveTask1 {
  int8_t   dir;   // 1-forvard, 0-no, -1 - backward
  int step_rest;  // downcount of next
  int step_task;  // total ticks in this task
  int d;          // for Brese
  inline void init() { dir = 0; step_rest = step_task = d = 0; }
};

extern MoveTask1 move_task[n_motors+1]; // last idx = time


#endif

