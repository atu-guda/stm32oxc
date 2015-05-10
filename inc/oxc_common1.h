#ifndef _OXC_COMMON1
#define _OXC_COMMON1

#include <oxc_devio.h>

// SmallRL storage and config
int smallrl_print( const char *s, int l );
int smallrl_exec( const char *s, int l );
void smallrl_sigint(void); // unused?
void sigint(int v);


extern "C" {

void task_leds( void *prm UNUSED_ARG );
void task_gchar( void *prm UNUSED_ARG );

}




#endif

