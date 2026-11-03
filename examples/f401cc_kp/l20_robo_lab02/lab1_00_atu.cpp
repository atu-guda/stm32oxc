#include <cstdint>
#include <cmath>

#include "lab1_common.h"

static int fun_idx { 0 };

int lab1_init( int x )
{
  fun_idx = x;
  return 0;
}

int lab1_step( uint32_t tc, float *alps )
{
  if ( tc > t_lab_max ) {
    return 1;
  }

  float t = (float) tc * 1e-3f;

  float ph_0 = t / T_0;
  float ph_1 = t / T_1;
  float q_0 = a_0 * sinf( ph_0 );
  float q_1 = a_1 * sinf( ph_1 );
  alps[0] = q_0;
  alps[1] = q_1;
  return 0;

}


