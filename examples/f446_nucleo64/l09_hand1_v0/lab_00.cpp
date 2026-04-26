#include <cmath>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_easing.h>

#include "main.h"


int lab_init( int x )
{
  return 0;
}

int lab_step( uint32_t tc )
{
  if( tc < (uint32_t)t_lab_max ) {
    set_l0_v( 0.2 );
    return 0;
  }
  return 1;
}


