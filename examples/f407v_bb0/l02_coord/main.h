#ifndef _MAIN_H
#define _MAIN_H

const inline uint32_t  TIM3_base_freq  { 84'000'000 };
const inline uint32_t  TIM3_count_freq {      10000 };
const inline uint32_t  TIM6_base_freq  {  1'000'000 };
const inline uint32_t  TIM6_count_freq {      20000 };

// mech params
struct MechParam {
  uint32_t tick2mm   ; // tick per mm, = 2* pulses per mm
  uint32_t max_speed ; // mm/min
  uint32_t max_l     ; // mm
  PinsIn  *endstops  ;
  PinsOut *motor     ;
};

const inline constinit unsigned n_motors { 5 };

extern MechParam mechs[n_motors];

struct MechState {
  float x[n_motors];
  uint32_t n_mo { 0 }; // current number of active motors
  uint32_t last_rc;
  bool was_set { false };
};

extern MechState me_st;

// move task description
struct MoveTask1 {
  int8_t   dir;   // 1-forvard, 0-no, -1 - backward
  int step_rest;  // downcount of next
  int step_task;  // total ticks in this task
  int d;          // for Brese
  inline void init() { dir = 0; step_rest = step_task = d = 0; }
};

extern MoveTask1 move_task[n_motors+1]; // last idx = time

const inline unsigned n_pow_ch { 3 };

#endif

