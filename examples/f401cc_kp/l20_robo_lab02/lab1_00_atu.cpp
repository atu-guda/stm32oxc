#include <cstdint>
#include <cmath>

#include "main.h"

#include "lab1_common.h"

constexpr size_t easing_fun_max { 6 }; // ignore step
static size_t fun_idx0 { 4 };
static size_t fun_idx1 { 4 };

struct PhAndSign {
  float ph;
  int sign;
};

int lab1_init( int x )
{
  fun_idx0 = std::clamp( (size_t)(x&0x0F),      0uz, easing_fun_max );
  fun_idx1 = std::clamp( (size_t)((x>>4)&0x0F), 0uz, easing_fun_max );
  return 0;
}

PhAndSign t2ph( float t, float t_0 )
{
  int sgn = 1;
  float dummy;
  float ph = 4 * modff( t / t_0, &dummy );
  switch( (unsigned)( ph ) ) {
    case 0: break;
    case 1: ph = 2.0f - ph; break;
    case 2: ph = ph - 2.0f; sgn = -1; break;
    case 3: ph = 4.0f - ph; sgn = -1; break;
    default: ph = 0;
  }
  return PhAndSign{ ph, sgn };
}

int lab1_step( uint32_t tc, float *alps )
{
  if ( tc > t_lab_max ) {
    return 1;
  }

  float t = (float) tc * 1e-3f;

  auto [ ph_0, sgn_a0 ] = t2ph( t, T_0 );
  auto [ ph_1, sgn_a1 ] = t2ph( t, T_1 );

  pha_0 = ph_0; pha_1 = ph_1; // debug?

  float q_0 = sgn_a0 * a_0 * (easing_fun_info[fun_idx0].f)( ph_0 );
  float q_1 = sgn_a1 * a_1 * (easing_fun_info[fun_idx1].f)( ph_1 );

  alps[0] = q_0;
  alps[1] = q_1;
  return 0;

}


