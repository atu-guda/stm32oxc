/*
 *    Description:  common declarations for pwm ctl functions (1ch)
 */

#ifndef _PWM1_CTL_H
#define _PWM1_CTL_H

inline const unsigned max_pwm_steps = 32;

struct StepInfo {
  float v;
  int t, tp;
};

struct PWMData {
  float val = 10.0f;    // unrestricted
  float val_1 = val;    // w/o hand
  float val_r = val;    // real
  float vmin = 10.0f, vmax = 60.0f;
  float hand = 0;       // adjusted by hand (handle_keys)

  unsigned n_steps  = 0;
  int t = 0;
  float t_mul = 1;
  StepInfo steps[max_pwm_steps];
};

extern PWMData pwmdat;

void pwm_reset_steps( PWMData &d );
void pwm_mk_rect( PWMData &d, float v, int t );
void pwm_mk_ladder( PWMData &d, float dv, int dt, unsigned n_up );
void pwm_mk_trap( PWMData &d,  float v, int t1, int t2, int t3 );
void pwm_show_steps( const PWMData &d );

int cmd_set_minmax( int argc, const char * const * argv );
int cmd_show_steps( int argc, const char * const * argv );
int cmd_mk_rect( int argc, const char * const * argv );
int cmd_mk_ladder( int argc, const char * const * argv );
int cmd_mk_trap( int argc, const char * const * argv );
int cmd_edit_step( int argc, const char * const * argv );

extern CmdInfo CMDINFO_SET_MINMAX;
extern CmdInfo CMDINFO_SHOW_STEPS;
extern CmdInfo CMDINFO_MK_RECT;
extern CmdInfo CMDINFO_MK_LADDER;
extern CmdInfo CMDINFO_MK_TRAP;
extern CmdInfo CMDINFO_EDIT_STEP;

#define CMDINFOS_PWM &CMDINFO_SET_MINMAX,  &CMDINFO_SHOW_STEPS,  &CMDINFO_MK_RECT,  &CMDINFO_MK_LADDER,  &CMDINFO_MK_TRAP, &CMDINFO_EDIT_STEP

#endif

