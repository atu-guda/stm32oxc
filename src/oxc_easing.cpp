#include <cmath>
#include <algorithm>

#include <oxc_easing.h>

static constexpr float pi_f = std::numbers::pi_v<float>;
static constexpr float pi_half_f = pi_f / 2;


float easing_one( float x )
{
  return easing_lim( x );
}

float easing_poly2_in( float x )
{
  x = easing_lim( x );
  return x*x;
}

float easing_poly2_out( float x )
{
  x = easing_lim( x );
  return x * ( 2 - x );
}

float easing_poly3_io( float x )
{
  x = easing_lim( x );
  return -2 * x*x*x + 3*x*x;
}

float easing_trig_in(  float x )
{
  x = easing_lim( x );
  return 1 - cosf( x * pi_half_f );
}

float easing_trig_out(  float x )
{
  x = easing_lim( x );
  return sinf( x * pi_half_f );
}

float easing_trig_io(  float x )
{
  x = easing_lim( x );
  return 0.5f * ( 1 - cosf( pi_f * x ) );
}

float easing_step_01(  float x )
{
  x = easing_lim( x );
  return ( x > 0.1f ) ? 1.0f : 0.0f;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

