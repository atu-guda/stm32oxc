/*
 *    Description:  common functions to control PWM on 1 ch
 *        Version:  1.0
 *        Created:  2019.04.06 13:05 as copy of pwm1_ctl.cpp
*/
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

#include <../examples/common/inc/pwm2_ctl.h>

using namespace std;


void PWMData::reset_steps()
{
  for( auto &s : steps ) {
    s.vb = s.ve = pwm_def; s.t = 30000; s.tp = pwm_type::pwm;
  }
  n_steps = 3;
}

void PWMData::mk_rect( float v, int t )
{
  const auto b = pwm_def;
  steps[0].vb = steps[0].ve = b; steps[0].t = 10000; steps[0].tp = pwm_type::pwm;
  steps[1].vb = steps[1].ve = v; steps[1].t = t;     steps[1].tp = pwm_type::pwm;
  steps[2].vb = steps[2].ve = b; steps[2].t = 30000; steps[2].tp = pwm_type::pwm;
  n_steps = 3;
}

void PWMData::mk_ladder( float dv, int t, unsigned n_up )
{
  unsigned n_up_max = max_pwm_steps / 2 - 1;
  n_up = clamp( n_up, 1u, n_up_max );

  const auto b = pwm_def;

  steps[0].vb =  steps[0].ve = b; steps[0].t = 10000; steps[0].tp = pwm_type::pwm;
  unsigned i = 1;
  float cv = b;
  for( /* NOP */; i <= n_up; ++i ) {
    cv += dv;
    steps[i].vb = steps[i].ve = cv;
    steps[i].t = t; steps[i].tp = pwm_type::pwm;
  }
  for( /* NOP */; i < n_up*2; ++i ) {
    cv -= dv;
    steps[i].vb = steps[i].ve = cv;
    steps[i].t = t; steps[i].tp = pwm_type::pwm;
  }
  steps[i].vb = steps[i].ve = b; steps[i].t = 60000; steps[0].tp = pwm_type::pwm;
  n_steps = n_up * 2 + 1;
}

void PWMData::mk_ramp( float v, int t1, int t2, int t3 )
{
  const auto b = pwm_def;
  steps[0].vb = b; steps[0].ve = b; steps[0].t = 30000; steps[0].tp = pwm_type::pwm;
  steps[1].vb = b; steps[1].ve = v; steps[1].t = t1;    steps[1].tp = pwm_type::pwm;
  steps[2].vb = v; steps[2].ve = v; steps[2].t = t2;    steps[2].tp = pwm_type::pwm;
  steps[3].vb = v; steps[3].ve = b; steps[3].t = t3;    steps[3].tp = pwm_type::pwm;
  steps[4].vb = b; steps[4].ve = b; steps[4].t = 30000; steps[4].tp = pwm_type::pwm;
  n_steps = 5;
}

void PWMData::show_steps() const
{
  STDOUT_os;
  os << "# pwm_min= " << pwm_min << "  pwm_def= " << pwm_def << "  pwm_max= " << pwm_max << " n_steps= " << n_steps << NL;
  int tc = 0;
  for( unsigned i=0; i<n_steps; ++i ) {
    os << "# [" << i << "] " << tc << ' ' << steps[i].t << ' '
       << steps[i].vb << ' ' << steps[i].ve << ' ' << (int)steps[i].tp << NL;
    tc += steps[i].t;
  }
  os << "# Total: " << tc << " ms" NL;
}

bool PWMData::edit_step( unsigned ns, float vb, float ve, int t, int tp )
{
  if( ns >= max_pwm_steps ) {
    return false;
  }
  steps[ns].vb = vb; steps[ns].vb = ve; steps[ns].t = t;
  if( tp >= (int)(pwm_type::n) ) {
    tp = 0;
  }
  steps[ns].tp = static_cast<pwm_type>(tp);
  if( ns >= n_steps ) {
    n_steps = ns+1;
  }
  return true;
}

void PWMData::set_pwm()
{
  pwm_r = clamp( pwm_val, pwm_min, pwm_max );

  uint32_t scl = tim_h.Instance->ARR; // TODO: external fun (ptr)
  using tim_ccr_t = decltype( tim_h.Instance->CCR1 );
  tim_ccr_t nv = (tim_ccr_t)( pwm_r * scl / 100 );
  if( nv != tim_h.Instance->CCR1 ) {
    tim_h.Instance->CCR1 = nv;
  }
}

void PWMData::prep( int a_t_step, bool fake )
{
  t_step = a_t_step; fake_run = fake;
  t = 0; t_mul = 1;  c_step = 0; hand = 0;
  calcNextStep();
  if( ! fake_run ) {
    pwm_r = val;
  }
}

bool PWMData::tick( const float * /*d*/ )
{
  t += t_step * t_mul;
  if( fake_run ) {
    return true;
  }

  if( t >= step_t ) { // next step
    t = 0;
    ++c_step;
    if( c_step >= n_steps ) {
      return false;
    }
    calcNextStep();
  }

  val_1 = val_0 + ks * t;
  val = val_1 + hand;
  pwm_val = val;  // TODO: switch
  set_pwm();

  return true;
}

void PWMData::calcNextStep()
{
  val_0  = steps[c_step].vb; val = val_0;
  step_t = steps[c_step].t;
  ks = ( steps[c_step].ve - val_0 ) / step_t;
}

void PWMData::end_run()
{
  if( ! fake_run ) {
    pwm_val = pwm_def; hand = 0; t = 0; c_step = 0;
    set_pwm();
  }
}

void PWMData::set_pwm_manual( float v )
{
  pwm_val = v; hand = 0;
  set_pwm();
}

// --------------------------- Commands -------------------------------------

int cmd_show_steps( int /*argc*/, const char * const * /*argv*/ )
{
  pwmdat.show_steps();
  return 0;
}


int cmd_mk_rect( int argc, const char * const * argv )
{
  float v  = arg2float_d( 1, argc, argv,    35, 1,       98 );
  int   t  = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  STDOUT_os;
  os << "# rect: v= " << v << " t= " << t << NL;
  pwmdat.mk_rect( v, t );
  pwmdat.show_steps();
  return 0;
}

int cmd_mk_ladder( int argc, const char * const * argv )
{
  float    v  = arg2float_d( 1, argc, argv,     5, 1,       90 );
  unsigned t  = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  unsigned n  = arg2long_d(  3, argc, argv,    10, 1, PWMData::max_pwm_steps/2-2 );
  STDOUT_os;
  os << "# ladder: v= " << v << " t= " << t << " n= " << n << NL;
  pwmdat.mk_ladder( v, t, n );
  pwmdat.show_steps();
  return 0;
}

int cmd_mk_ramp( int argc, const char * const * argv )
{
  float  v = arg2float_d( 1, argc, argv,    50, 0,       99 );
  int   t1 = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  int   t2 = arg2long_d(  3, argc, argv, 30000, 1, 10000000 );
  int   t3 = arg2long_d(  4, argc, argv, 30000, 1, 10000000 );
  STDOUT_os;
  os << "# ramp: v= " << v << " t1= " << t1 << " t2= " << t2 << " t3= " << t3  << NL;
  pwmdat.mk_ramp( v, t1, t2, t3 );
  pwmdat.show_steps();
  return 0;
}

int cmd_edit_step( int argc, const char * const * argv )
{
  if( argc < 4 ) {
    return 1;
  }
  unsigned j = arg2long_d(  1, argc, argv,     0, 0, PWMData::max_pwm_steps-1 );
  float vb   = arg2float_d( 2, argc, argv,    25, 0, 10000 );
  float ve   = arg2float_d( 2, argc, argv,    25, 0, 10000 );
  int   t    = arg2long_d(  4, argc, argv, 30000, 1, 10000000 );
  int   tp   = arg2long_d(  5, argc, argv,     0, 0, 1 );
  bool ok = pwmdat.edit_step( j, vb, ve, t, tp );
  pwmdat.show_steps();
  return ok ? 0 : 1;
}

CmdInfo CMDINFO_SHOW_STEPS { "show_steps", 'S', cmd_show_steps, " - show PWM steps"  };
CmdInfo CMDINFO_MK_RECT    { "mk_rect",      0, cmd_mk_rect,    " v t - make rectangle steps"  };
CmdInfo CMDINFO_MK_LADDER  { "mk_ladder",    0, cmd_mk_ladder,  " v t n_up - make ladder steps"  };
CmdInfo CMDINFO_MK_RAMP    { "mk_ramp",      0, cmd_mk_ramp,    " v t1  t2 t3 - make ramp steps"  };
CmdInfo CMDINFO_EDIT_STEP  { "edit_step",  'E', cmd_edit_step,  " vb ve t tp - edit given step"  };

