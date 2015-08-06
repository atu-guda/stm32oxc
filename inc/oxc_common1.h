#ifndef _OXC_COMMON1
#define _OXC_COMMON1

#include <oxc_devio.h>

#ifndef TASK_LEDS_QUANT
  #define TASK_LEDS_QUANT 100
#endif
// delay is TASK_LEDS_QUANT * task_leds_step,
extern volatile int task_leds_step; // initial = 5

// SmallRL storage and config
int smallrl_print( const char *s, int l ) __weak;
int smallrl_exec( const char *s, int l ) __weak;
void smallrl_sigint(void) __weak; // unused?
void sigint(int v) __weak;


extern "C" {

void task_leds( void *prm UNUSED_ARG ) __weak;
void task_gchar( void *prm UNUSED_ARG ) __weak;

}




#endif

