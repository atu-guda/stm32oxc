/*
 *    Description:  common functions to control PWM on 1 ch
 *        Version:  1.0
 *        Created:  2019.04.06 13:05 as copy of pwm1_ctl.cpp
*/
#include <cmath>
#include <algorithm>
#include <iterator>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

#include <../examples/common/inc/pwm2_ctl.h>

using namespace std;

PWMInfo::PWMInfo( float a_R0, float a_V_00, float a_k_gv1 )
  : R_0( a_R0 ), V_00( a_V_00 ), k_gv1( a_k_gv1 )
{
  fixCoeffs();
  fillFakeCalibration( 10 );
}

void PWMInfo::fixCoeffs()
{
  k_gv2 = -k_gv1 * k_gv1 / ( 4 * V_00 );
  x_0  =  - V_00 / k_gv1;
}

float PWMInfo::hint_for_V( float V )
{
  if( need_regre ) { // TODO: move to separate fun + restore const
    float a, b, r;
    bool ok = regreCalibration( x_0, a, b, r );
    if( ok ) {
      k_gv1 = a; V_00 = b;
      fixCoeffs();
    }
  }

  return ( V > -V_00 )
    ? ( ( V - V_00 ) / k_gv1 )
    : sqrtf( V / k_gv2 );
};

float PWMInfo::pwm2V( float pwm ) const
{
  return ( pwm > 2 * x_0 )
    ? ( pwm * k_gv1 + V_00 )
    : ( pwm * pwm * k_gv2 );
}

void PWMInfo::clearCalibrationArr()
{
  n_cal = 0;
  was_calibr = false; // as fake;
  for( unsigned i=0; i<max_cal_steps; ++i ) {
    d_pwm[i] = 0;
    d_v[i]   = 0;
    d_i[i]   = 0;
  }
  need_regre = true;
}

void PWMInfo::fillFakeCalibration( unsigned nc )
{
  if( nc > max_cal_steps ) {
    nc = max_cal_steps;
  }
  for( unsigned i=0; i<nc; ++i ) {
    float pwm = cal_min + cal_step * i;
    d_pwm[i] = pwm;
    d_v[i]   = pwm2V( pwm );
    d_i[i]   = d_v[i] / R_0;
  }
  n_cal = nc;
  was_calibr = false; // as fake;
  need_regre = true;
}

void PWMInfo::addCalibrationStep( float pwm, float v, float I )
{
  if( n_cal >= max_cal_steps ) {
    return;
  }
  d_pwm[n_cal] = pwm;
  d_v[n_cal]   = v;
  d_i[n_cal]   = I;
  ++n_cal;
}

bool  PWMInfo::calcCalibration( float &err_max, bool fake )
{
  std_out << "# -- calibration calculation ----- " ;
  if( fake ) {
    std_out << " === FAKE ===";
  }
  std_out << NL;

  if( n_cal < 3 || fabsf( d_i[0] < 1e-4f ) ) {
    std_out << "# Error: bad input data " NL;
    return false;
  }

  float t_R_0 = d_v[0] / d_i[0];

  // find initial mode change
  unsigned i_lim = n_cal;
  float k_g = 0.12f;
  for( unsigned i=n_cal-1; i>0; --i ) {
    float diff_pwm_l =  d_pwm[i]       - d_pwm[i-1];
    float diff_pwm_g =  d_pwm[n_cal-1] - d_pwm[i-1];
    std_out << "# " << i << ' ';
    if( fabsf( diff_pwm_l ) < 0.1f || fabsf( diff_pwm_g ) < 0.1f ) {
      continue;
    }
    float k_l1 = ( d_v[i]       - d_v[i-1] ) / diff_pwm_l;
    float k_g1 = ( d_v[n_cal-1] - d_v[i-1] ) / diff_pwm_g;
    if( fabsf( ( k_g1 - k_l1 ) / k_g1 ) < 0.03f ) {
      i_lim = i; k_g = k_g1;
    }
    std_out << ' ' << i << ' ' << k_l1 << ' ' << k_g1 << NL;
  }

  float b = d_v[n_cal-1] - k_g * d_pwm[n_cal-1];
  float t_x_0 = - b / k_g;

  float a_regr = 0, b_regr = 0, r_regr;
  bool ok_regr = regreCalibration( t_x_0, a_regr, b_regr, r_regr );
  std_out << "# regre: a= " << a_regr << " b= " << b_regr << " r= " << r_regr << " ok= " << ok_regr << NL;
  if( ! ok_regr ) {
    return false;
  }
  b = b_regr; k_g = a_regr;
  t_x_0 = - b / k_g;
  float k_2 = - k_g * k_g / ( 4 * b );

  std_out << "# --- calibration check:" << NL;

  err_max = 0;
  for( unsigned i=0; i<n_cal; ++i ) {
    float pwm = d_pwm[i];
    float v = ( pwm > 2 * t_x_0 ) ? ( k_g * pwm + b ) : (  pwm * pwm * k_2 );
    float err = fabsf( v - d_v[i] );
    if( err > err_max ) {
      err_max = err;
    }
    std_out << "# " << i << ' ' << pwm << ' ' << v << err << NL;
  }

  std_out << "# err_max= " << err_max << NL;
  std_out << "# R_0= " << t_R_0 << " i_lim= " << i_lim << " k_gv1= " << k_g
          << " V_00= " << b << " x_0= " << t_x_0 << " k_gv2= " << k_2 << NL;

  if( err_max > 0.3f ) {
    std_out << "# Error: large approximation error!!! " << NL;
    return false;
  }

  R_0   = t_R_0;
  V_00  = b;
  k_gv1 = k_g;
  k_gv2 = k_2;
  x_0   = t_x_0;

  if( !fake ) {
    was_calibr = true;
  }
  return true;
}

bool PWMInfo::regreCalibration( float t_x0, float &a, float &b, float &r )
{
  need_regre = true;
  unsigned n = 0;
  float sx = 0, sy = 0, sx2 = 0, sy2 = 0, sxy = 0;

  for( unsigned i=0; i<n_cal; ++i ) {
    if( d_pwm[i] < 2 * t_x0 ) {
      continue;
    }
    float x = d_pwm[i], y = d_v[i];
    sx  += x;
    sx2 += x * x;
    sy  += y;
    sy2 += y * y;
    sxy += x * y;
    ++n;
  }

  if( n < 2 ) {
    // std_out << "# Error: regre: n= " << n << NL;
    return false;
  }

  float dd = n * sx2 - sx * sx;
  if( fabsf( dd ) < 1e-6f ) {
    // std_out << "# Error: regre: dd= " << dd << NL;
    return false;
  }
  const float t1 = n * sxy - sx * sy;
  a = t1 / dd;
  b = ( sy * sx2 - sx * sxy ) / dd;

  const float dz = ( n * sx2 - sx * sx ) * ( n * sy2 - sy * sy );
  if( dz < 1e-6f ) {
    return false;
  }
  r = t1 / sqrtf( dz );

  //std_out << "###  regre: r= " << r << NL;
  if( r < 0.5f ) {
    // std_out << "# Error: regre: r= " << r << NL;
    return false;
  }

  need_regre = false;
  return true;
}

// ------------------------ PWMData -------------------------------------

void PWMData::reset_steps()
{
  for( auto &s : steps ) {
    s.vb = s.ve = pwm_def; s.t = 30000; s.tp = pwm_type::pwm;
  }
  n_steps = 0;
}

int PWMData::add_step( float b, float e, int ms, pwm_type tp )
{
  if( n_steps >= size(steps) ) {
    return 0;
  }

  steps[n_steps].vb = b;
  steps[n_steps].ve = e;
  steps[n_steps].t  = ms;
  steps[n_steps].tp = tp;

  ++n_steps;
  return n_steps;
}

void PWMData::mk_rect( float vmin, float vmax, int t, pwm_type tp )
{
  reset_steps();
  add_step( vmin, vmin, 10000, tp );
  add_step( vmax, vmax,     t, tp );
  add_step( vmin, vmin, 30000, tp );
}

void PWMData::mk_ladder( float v0, float dv, int t, unsigned n_up, pwm_type tp )
{
  reset_steps();
  unsigned n_up_max = max_pwm_steps / 2 - 2;
  n_up = clamp( n_up, 1u, n_up_max );

  add_step( v0, v0, 10000, tp );
  unsigned i = 1;
  float cv = v0;
  for( /* NOP */; i <= n_up; ++i ) {
    cv += dv;
    add_step( cv, cv, t, tp );
  }
  for( /* NOP */; i < n_up*2; ++i ) {
    cv -= dv;
    add_step( cv, cv, t, tp );
  }
  add_step( v0, v0, 60000, tp );
}

void PWMData::mk_ramp( float vmin, float vmax, int t1, int t2, int t3, pwm_type tp )
{
  reset_steps();
  add_step( vmin, vmin, 30000, tp );
  add_step( vmin, vmax,    t1, tp );
  add_step( vmax, vmax,    t2, tp );
  add_step( vmax, vmin,    t3, tp );
  add_step( vmin, vmin, 30000, tp );
}

void PWMData::show_steps() const
{
  std_out << "# pwm_min= " << pwm_min << "  pwm_def= " << pwm_def << "  pwm_max= " << pwm_max << " n_steps= " << n_steps << NL;
  int tc = 0;
  for( unsigned i=0; i<n_steps; ++i ) {
    std_out << "# [" << i << "] " << tc << ' ' << steps[i].t << ' '
       << steps[i].vb << ' ' << steps[i].ve << ' ' << (int)steps[i].tp << NL;
    tc += steps[i].t;
  }
  std_out << "# Total: " << tc << " ms" NL;
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
                            {
                              auto i_eff = max( d[didx_i], 0.1f );
                              pwm_val -= pwminfo.ki_v * t_step * err / i_eff;
                            }
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
  std_out << "# @@@ " << c_step << ' ' << old_val << ' ' << val << ' ' << pwm_val << ' '
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
  if( d[didx_r] > pwminfo.R_max  ||  d[didx_r] < 0.001f ) {
    // std_out << "# Error: limit hard R: " << d[didx_r] << NL;
    leds.set( BIT0 );
    return check_result::hard;
  }

  if( d[didx_v] > pwminfo.V_max ) {
    pwm_val *= 0.95f * pwminfo.V_max / d[didx_v];
    pwm_tmax = 0.99f * pwm_val;
    leds.set( BIT1 );
    // std_out << "# limit V: " << d[didx_v] << " pwm_tmax=" << pwm_tmax << NL;
    return check_result::soft;
  }
  if( d[didx_i] > pwminfo.I_max ) {
    pwm_val *= 0.95f * pwminfo.I_max / d[didx_i];
    pwm_tmax = 0.99f * pwm_val;
    // std_out << "# limit I: " << d[didx_i] << " pwm_tmax=" << pwm_tmax << NL;
    leds.set( BIT1 );
    return check_result::soft;
  }
  if( d[didx_w] > pwminfo.W_max ) {
    pwm_val *= 0.95f * pwminfo.W_max / d[didx_w];
    pwm_tmax = 0.99f * pwm_val;
    // std_out << "# limit W: " << d[didx_w] << " pwm_tmax=" << pwm_tmax << NL;
    leds.set( BIT1 );
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
  std_out << "# rect: vmin= " << vmin << " vmax= " << vmax << " t= " << t << " tp= " << tp << NL;
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
  std_out << "# ladder: v0= " << v0 << " dv= " << dv << " t= " << t << " tp= " << tp << NL;
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
  std_out << "# ramp: vmin= " << vmin << " vmax= " << vmax
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
CmdInfo CMDINFO_MK_RAMP    { "mk_ramp",      0, cmd_mk_ramp,    " vmin vmax t1  t2 t3  tp - make ramp steps"  };
CmdInfo CMDINFO_EDIT_STEP  { "edit_step",  'E', cmd_edit_step,  " vb ve t tp - edit given step"  };

