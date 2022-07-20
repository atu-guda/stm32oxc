#include <oxc_stepmotor.h>

void StepMotor::setMode( size_t a_mode ) noexcept
{
  mode    = (mode<n_modes)? a_mode : 0;
  steps   = m_modes[mode].steps;
  n_steps = m_modes[mode].n_steps;
  ph = 0;
}

void StepMotor::setExternMode( const uint16_t *st, size_t ns ) noexcept
{
  steps   = st;
  n_steps = ns;
  ph = 0;
  mode = 0xFF; // fake
}

void StepMotor::step( int v )
{
  ph += (size_t)v;
  ph %= n_steps;
  drv.set( steps[ph] );
}


