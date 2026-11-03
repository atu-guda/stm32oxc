#include <cstdint>
#include <cmath>

#include "main.h"

#include "lab1_common.h"

constexpr size_t easing_fun_max { 6 }; // ignore step
static size_t fun_idx { 4 };

int lab1_init( int x )
{
  fun_idx = std::clamp( (size_t)x, 0uz, easing_fun_max );
  return 0;
}

int lab1_step( uint32_t tc, float *alps )
{
  if ( tc > t_lab_max ) {
    return 1;
  }

  float t = (float) tc * 1e-3f;

  float dummy;

  int sgn_a0 = 1;
  float ph_0 = 4 * modff( t / T_0, &dummy );
  // pha_1 = ph_0; // debug, TODO: remove
  auto part_0 = (unsigned)( ph_0 );
  switch( part_0 ) {
    case 0: break;
    case 1: ph_0 = 2.0f - ph_0; break;
    case 2: ph_0 = ph_0 - 2.0f; sgn_a0 = -1; break;
    case 3: ph_0 = 4.0f - ph_0; sgn_a0 = -1; break;
    default: ph_0 = 0;
  }

  int sgn_a1 = 1;
  float ph_1 = 4 * modff( t / T_1, &dummy );
  auto part_1 = (unsigned)( ph_1 );
  switch( part_1 ) {
    case 0: break;
    case 1: ph_1 = 2.0f - ph_1; break;
    case 2: ph_1 = ph_1 - 2.0f; sgn_a1 = -1; break;
    case 3: ph_1 = 4.0f - ph_1; sgn_a1 = -1; break;
    default: ph_1 = 0;
  }

  pha_0 = ph_0; pha_1 = ph_1;

  float q_0 = sgn_a0 * a_0 * (easing_fun_info[fun_idx].f)( ph_0 );
  float q_1 = sgn_a1 * a_1 * (easing_fun_info[fun_idx].f)( ph_1 );

  alps[0] = q_0;
  alps[1] = q_1;
  return 0;

}


