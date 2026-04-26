#ifndef _LAB0_COMMON_H
#define _LAB0_COMMON_H

// copy from oxc_easing.h
using EasingFun = float (*)(float); // [0..1] -> [0..1]
                                    //
extern const EasingFun easing_funcs[];
const inline size_t    easing_max { 8 };

struct TaskPart {
  float      q_e; //* final coordinate value
  uint32_t     t; //* time for part  (in ms)
  uint32_t    tp; //* type of easing function (index in easing_funcs )
  uint32_t   flg; //* misc flags
};

// description see at main.cpp
extern uint32_t measure_tick;
extern uint32_t measure_idle_step;
extern int      t_lab_max;
extern int      t_pre;
extern int      t_post;
extern int      t_meas;
extern int      t_step;
extern int      have_magn;
extern int      stopsw;
extern int      l0_freq;
extern int      l0_freq_min;
extern int      l0_freq_max;
extern int      l0_freq_n;
extern int      l0_v_n;
extern int      q0_i;
extern float    q0;
extern float    q0_g;
extern float    q0_0;
extern float    q0_emax;
extern float    v0_def;
extern float    v0_min;
extern float    nu0;

void set_l0_freq( uint32_t freq );
void set_l0_pwm( float pwm );
void set_l0_mode( bool i1, bool i2 );
void set_l0_v( float v ); // +-1
float get_l0_v();

int lab_init( int x ); //* x - first argument (at will), returns 0 - ok, >0 - error, >1 - emerg. stop
int lab_step( uint32_t tc ); //* tc - current time in ms from start, returns:0 - next, 1 - end, > 1 - err + end

#endif

