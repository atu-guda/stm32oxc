/*
 *    Description:  common functions to control PWM on 1 ch
 *        Version:  1.0
 *        Created:  2019.04.06 13:05 as copy of pwm1_ctl.cpp
*/
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

#include <../examples/common/inc/pwm2_ctl.h>

using namespace std;

float PWMInfo::hint_for_V( float V ) const {
  return ( V > -V_00 ) ? ( ( V - V_00 ) / k_gv1 ) : sqrtf( V / k_gv2 ) ;
};

void PWMData::reset_steps()
{
  for( auto &s : steps ) {
    s.vb = s.ve = pwm_def; s.t = 30000; s.tp = pwm_type::pwm;
  }
  n_steps = 1;
}

void PWMData::mk_rect( float vmin, float vmax, int t, pwm_type tp )
{
  steps[0].vb = steps[0].ve = vmin; steps[0].t = 10000; steps[0].tp = tp;
  steps[1].vb = steps[1].ve = vmax; steps[1].t = t;     steps[1].tp = tp;
  steps[2].vb = steps[2].ve = vmin; steps[2].t = 30000; steps[2].tp = tp;
  n_steps = 3;
}

void PWMData::mk_ladder( float v0, float dv, int t, unsigned n_up, pwm_type tp )
{
  unsigned n_up_max = max_pwm_steps / 2 - 1;
  n_up = clamp( n_up, 1u, n_up_max );

  steps[0].vb =  steps[0].ve = v0; steps[0].t = 10000; steps[0].tp = tp;
  unsigned i = 1;
  float cv = v0;
  for( /* NOP */; i <= n_up; ++i ) {
    cv += dv;
    steps[i].vb = steps[i].ve = cv;
    steps[i].t = t; steps[i].tp = tp;
  }
  for( /* NOP */; i < n_up*2; ++i ) {
    cv -= dv;
    steps[i].vb = steps[i].ve = cv;
    steps[i].t = t; steps[i].tp = tp;
  }
  steps[i].vb = steps[i].ve = v0; steps[i].t = 60000; steps[i].tp = tp;
  n_steps = n_up * 2 + 1;
}

void PWMData::mk_ramp( float vmin, float vmax, int t1, int t2, int t3, pwm_type tp )
{
  steps[0].vb = vmin; steps[0].ve = vmin; steps[0].t = 30000; steps[0].tp = tp;
  steps[1].vb = vmin; steps[1].ve = vmax; steps[1].t = t1;    steps[1].tp = tp;
  steps[2].vb = vmax; steps[2].ve = vmax; steps[2].t = t2;    steps[2].tp = tp;
  steps[3].vb = vmax; steps[3].ve = vmin; steps[3].t = t3;    steps[3].tp = tp;
  steps[4].vb = vmin; steps[4].ve = vmin; steps[4].t = 30000; steps[4].tp = tp;
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

bool PWMData::edit_step( unsigned ns, float vb, float ve, int t, pwm_type tp )
{
  if( ns >= max_pwm_steps ) {
    return false;
  }
  steps[ns].vb = vb; steps[ns].ve = ve; steps[ns].t = t;
  if( tp >= pwm_type::n ) {
    tp = pwm_type::pwm;
  }
  steps[ns].tp = static_cast<pwm_type>(tp);
  if( ns >= n_steps ) {
    n_steps = ns+1;
  }
  return true;
}

void PWMData::set_pwm()
{
  pwm_r = clamp( pwm_val, pwm_min, pwm_tmax );
  if( set_pwm_real ) {
    set_pwm_real( pwm_r );
  }

}

void PWMData::prep( int a_t_step, bool fake )
{
  pwm_tmax = pwm_max;
  t_step = a_t_step; fake_run = fake;
  t = 0; t_mul = 1;  c_step = 0; hand = 0; last_R = pwminfo.R_0;
  calcNextStep();
  if( ! fake_run ) {
    pwm_r = val;
  }
}

// TODO: return reason
bool PWMData::tick( const float *d )
{
  t += t_step * t_mul;
  last_R = d[didx_r];
  auto pwm_val_old = pwm_val;

  auto rc = check_lim( d );

  if( rc == check_result::hard ) {
    return false;
  }

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
  } else {

    val_1 = val_0 + ks * t;
    val = val_1 + hand;
    float err;
    switch( steps[c_step].tp ) {
      case pwm_type::pwm:   pwm_val = val; break;
      case pwm_type::volt:  err = d[didx_v] - val;
                            pwm_val -= pwminfo.ki_v * t_step * err;
                            break;
      case pwm_type::curr:  err = d[didx_i] - val;
                            pwm_val -= pwminfo.ki_v * last_R * t_step * err;
                            break;
      case pwm_type::pwr:   err = d[didx_w] - val;
                            pwm_val -= pwminfo.ki_v * t_step * err / ( d[1] + 0.2f ); // TODO: * what? / d[1]
                            break;
      default:              pwm_val = pwm_min; break; // fail-save
    };
  }
  if( rc != check_result::ok && pwm_val > pwm_val_old ) {
    pwm_val = pwm_val_old;
  }

  pwm_val = clamp( pwm_val, pwm_min, pwm_tmax ); // to no-overintegrate
  set_pwm();

  return true;
}

void PWMData::calcNextStep()
{
  float old_val = val;
  val_0  = steps[c_step].vb; val = val_0;
  step_t = steps[c_step].t;
  ks = ( steps[c_step].ve - val_0 ) / step_t;
  bool rehint = ( c_step == 0  )
    || ( fabsf( val_0 - old_val ) > pwminfo.rehint_lim )
    || ( steps[c_step].tp != steps[c_step-1].tp );
  switch( steps[c_step].tp ) {
    case pwm_type::pwm:   pwm_val = val; break;
    case pwm_type::volt:  if( rehint ) {
                            pwm_val = pwminfo.hint_for_V( val );
                          }
                          break;
    case pwm_type::curr:  if( rehint ) {
                            pwm_val = pwminfo.hint_for_V( val * last_R );
                          }
                          break;
    case pwm_type::pwr:  if( rehint ) {
                            pwm_val = pwminfo.hint_for_V( sqrtf( val * last_R  ) );
                          }
                          break;
    default:              pwm_val = pwm_min; break; // fail-save
  };
  STDOUT_os; // debug. TODO: remove
  os << "# @@@ " << c_step << ' ' << old_val << ' ' << val << ' ' << pwm_val << ' '
     << last_R << ' ' << rehint << NL;
}

void PWMData::end_run()
{
  if( ! fake_run ) {
    pwm_val = pwm_def; hand = 0; t = 0; c_step = 0;
    set_pwm();
  }
  pwm_tmax = pwm_max;
}

PWMData::check_result PWMData::check_lim( const float *d )
{
  STDOUT_os;
  if( d[didx_r] > pwminfo.R_max  ||  d[didx_r] < 0.001f ) {
    os << "# Error: limit hard R: " << d[didx_r] << NL;
    return check_result::hard;
  }

  if( d[didx_v] > pwminfo.V_max ) {
    pwm_val *= 0.95f * pwminfo.V_max / d[didx_v];
    pwm_tmax = 0.99f * pwm_val;
    os << "# limit V: " << d[didx_v] << " pwm_tmax=" << pwm_tmax << NL;
    return check_result::soft;
  }
  if( d[didx_i] > pwminfo.I_max ) {
    pwm_val *= 0.95f * pwminfo.I_max / d[didx_i];
    pwm_tmax = 0.99f * pwm_val;
    os << "# limit I: " << d[didx_i] << " pwm_tmax=" << pwm_tmax << NL;
    return check_result::soft;
  }
  if( d[didx_w] > pwminfo.W_max ) {
    pwm_val *= 0.95f * pwminfo.W_max / d[didx_w];
    pwm_tmax = 0.99f * pwm_val;
    os << "# limit W: " << d[didx_w] << " pwm_tmax=" << pwm_tmax << NL;
    return check_result::soft;
  }

  return check_result::ok;
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
  float vmin  = arg2float_d( 1, argc, argv,     5, 0.1f,       98 );
  float vmax  = arg2float_d( 2, argc, argv,    35, 0.2f,       98 );
  int   t     = arg2long_d(  3, argc, argv, 30000,    1, 10000000 );
  int   tp    = arg2long_d(  4, argc, argv,     0,    0, (int)(PWMData::pwm_type::n)-1 );
  STDOUT_os;
  os << "# rect: vmin= " << vmin << " vmax= " << vmax << " t= " << t << " tp= " << tp << NL;
  pwmdat.mk_rect( vmin, vmax, t, (PWMData::pwm_type)(tp) );
  pwmdat.show_steps();
  return 0;
}

int cmd_mk_ladder( int argc, const char * const * argv )
{
  float    v0  = arg2float_d( 1, argc, argv,     3, 0.1f,       90 );
  float    dv  = arg2float_d( 2, argc, argv,     5, 0.0f,       90 );
  unsigned t   = arg2long_d(  3, argc, argv, 30000,    1, 10000000 );
  unsigned n   = arg2long_d(  4, argc, argv,    10,    1, PWMData::max_pwm_steps/2-2 );
  int   tp     = arg2long_d(  5, argc, argv,     0,    0, (int)(PWMData::pwm_type::n)-1 );
  STDOUT_os;
  os << "# ladder: v0= " << v0 << " dv= " << dv << " t= " << t << " tp= " << tp << NL;
  pwmdat.mk_ladder( v0, dv, t, n, (PWMData::pwm_type)(tp) );
  pwmdat.show_steps();
  return 0;
}

int cmd_mk_ramp( int argc, const char * const * argv )
{
  float vmin  = arg2float_d( 1, argc, argv,     5, 0.1f,       98 );
  float vmax  = arg2float_d( 2, argc, argv,    35, 0.2f,       98 );
  int   t1    = arg2long_d(  3, argc, argv, 30000,    1, 10000000 );
  int   t2    = arg2long_d(  4, argc, argv, 30000,    1, 10000000 );
  int   t3    = arg2long_d(  5, argc, argv, 30000,    1, 10000000 );
  int   tp    = arg2long_d(  6, argc, argv,     0,    0, (int)(PWMData::pwm_type::n)-1 );
  STDOUT_os;
  os << "# ramp: vmin= " << vmin << " vmax= " << vmax
     <<  " t1= " << t1 << " t2= " << t2 << " t3= " << t3  << " tp= " << tp << NL;
  pwmdat.mk_ramp( vmin, vmax, t1, t2, t3, (PWMData::pwm_type)(tp) );
  pwmdat.show_steps();
  return 0;
}

int cmd_edit_step( int argc, const char * const * argv )
{
  if( argc < 4 ) {
    return 1;
  }
  unsigned j = arg2long_d(  1, argc, argv,     0, 0, PWMData::max_pwm_steps-1 );
  float vb   = arg2float_d( 2, argc, argv,     1, 0, 10000 );
  float ve   = arg2float_d( 3, argc, argv,     3, 0, 10000 );
  int   t    = arg2long_d(  4, argc, argv, 30000, 1, 10000000 );
  int   tp   = arg2long_d(  5, argc, argv,     0, 0, (int)(PWMData::pwm_type::n)-1 );
  bool ok = pwmdat.edit_step( j, vb, ve, t, (PWMData::pwm_type)(tp) );
  pwmdat.show_steps();
  return ok ? 0 : 1;
}

CmdInfo CMDINFO_SHOW_STEPS { "show_steps", 'S', cmd_show_steps, " - show PWM steps"  };
CmdInfo CMDINFO_MK_RECT    { "mk_rect",      0, cmd_mk_rect,    " vmin vmax t tp - make rectangle steps"  };
CmdInfo CMDINFO_MK_LADDER  { "mk_ladder",    0, cmd_mk_ladder,  " vmin dv t n_up tp - make ladder steps"  };
CmdInfo CMDINFO_MK_RAMP    { "mk_ramp",      0, cmd_mk_ramp,    " vmin vmax v t1  t2 t3  tp - make ramp steps"  };
CmdInfo CMDINFO_EDIT_STEP  { "edit_step",  'E', cmd_edit_step,  " vb ve t tp - edit given step"  };

