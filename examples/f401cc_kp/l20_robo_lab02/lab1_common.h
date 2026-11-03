#ifndef _LAB1_COMMON_H
#define _LAB1_COMMON_H

// description see at main.cpp
extern uint32_t      t_pre;
extern uint32_t      t_run_c;
extern uint32_t      t_post;
extern uint32_t      t_meas;
extern uint32_t      t_step;
extern uint32_t      t_lab_max;

extern float T_0;
extern float T_1;
extern float a_0;
extern float a_1;

int lab1_init( int x ); //* x - first argument (at will), returns 0 - ok, >0 - error, >1 - emerg. stop
int lab1_step( uint32_t tc, float *alps ); //* tc - current time in ms from start, returns:0 - next, 1 - end, > 1 - err + end

#endif

