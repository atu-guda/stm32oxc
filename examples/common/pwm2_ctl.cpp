/*
 *    Description:  common functions to control PWM on 1 ch
 *        Version:  1.0
 *        Created:  2019.04.06 13:05 as copy of pwm1_ctl.cpp
*/
#include <cmath>
#include <algorithm>
#include <iterator>

#include <oxc_auto.h>
#include <oxc_debug1.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <../examples/common/inc/pwm2_ctl.h>

using namespace std;

#define debug (UVAR('d'))

constexpr inline float pow2( float x ) { return x * x; }

PWMInfo::PWMInfo( float a_R0, float a_V_00, float a_k_gv1, float freq )
  : R_0( a_R0 ), V_00( a_V_00 ), k_gv1( a_k_gv1 ), T_0( 1.0f / freq )
{
  fixCoeffs();
  // fillFakeCalibration( 10 );
}

void PWMInfo::fixCoeffs()
{
  k_gv2 = -k_gv1 * k_gv1 / ( 4 * V_00 );
  x_0  =  - V_00 / k_gv1;
}


float PWMInfo::hint_for_V( float V ) const
{
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

float PWMInfo::estimateV( float pwm, float R_h ) const
{
  const float eps = 5.0e-2;
  const float gamma = 0.01f * pwm;
  float blr = L / ( R_h * T_0 ); // betta_{LRh}
  float V_p = V_cc;
  float V_dn = -0.48f; // V_dn0;
  const float gamma_m1 = 1.0f - gamma, gamma2 = pow2( gamma ), gamma4 = pow2( gamma2 );

  float dV_2 = 1000.0f;
  float oV_2 = 1000.0f;
  float V_2 = 0, I_h = 0, V_2_n0 = 0;
  bool is_CCM = false;

  for( int i=0; i<20 && fabsf(dV_2) > eps; ++i ) {

    float V_x =
      sqrtf(
          ( pow2(V_dn) - 2 * V_p * V_dn + pow2(V_p) ) * gamma4
          + ( 4 * pow2(V_dn) - 12 * V_p * V_dn + 8 * pow2(V_p) ) * blr * gamma2
          + 4 * pow2(V_dn) * pow2(blr)
          )
      + ( V_dn - V_p ) * gamma2 + 2 * V_dn * blr;

    float T_2 = T_0 * V_x / ( 2 * gamma * ( V_p - V_dn ) );
    float T_23 = T_0 * gamma_m1;
    is_CCM = false;
    if( T_2 > T_23 ) {
      is_CCM = true;
      T_2 = T_23;
    }

    if( is_CCM ) {
      V_2 = V_p * gamma + V_dn * gamma_m1;
    } else {
      V_2 = V_x / ( 4 * blr );
    }
    if( i == 0 ) {
      V_2_n0 = V_2;
    }
    if( V_2 < 0 ) {
      // std_out << "### Forced break!" << endl;
      V_2 = V_2_n0;
      oV_2 = V_2;
    }

    I_h = V_2 / R_h;
    V_dn = 0.7f * V_dn + 0.3f * V_dn_f( I_h ); // TODO: adj coeff
    if( debug > 1 ) {
      std_out << "# " << i << " V_2= " << V_2 << " I_h= " << I_h << " V_dn= " << V_dn
        << " V_p= " << V_p << " V_x= " << V_x << NL;
    }
    V_p = 0.5f * V_p + 0.5f * ( V_cc - R_ch * I_h ) ; // I_L ?
    dV_2 = V_2 - oV_2; oV_2 = V_2;
  }
  return V_2;
}

void PWMInfo::clearCalibrationArr()
{
  n_cal = 0;
  was_calibr = false; // as fake;
  for( unsigned i=0; i<max_cal_steps; ++i ) {
    d_pwm[i] = 0;
    d_v[i]   = 0;
    d_wei[i] = 0;
  }
  need_regre = true;
}

void PWMInfo::fillFakeCalibration( float R_h )
{
  unsigned nc = 0;
  for( unsigned i=0; i<max_cal_steps; ++i ) {
    float pwm = cal_min + cal_step * i;
    if( pwm > 99.5f ) {
      break;
    }
    d_pwm[i] = pwm;
    d_v[i]   = estimateV( pwm, R_h );
    d_wei[i] = min_cal_req;
    ++nc;
  }
  n_cal = nc;
  R_0 = R_h;
  was_calibr = false; // as fake;
  need_regre = true;
}

void PWMInfo::addCalibrationStep( float pwm, float v, float /* I */ )
{
  if( n_cal >= max_cal_steps ) {
    return;
  }
  d_pwm[n_cal] = pwm;
  d_wei[n_cal] = min_cal_req;
  d_v[n_cal]   = v;
  ++n_cal;
}

bool  PWMInfo::calcCalibration( float &err_max, float R_0_c,  bool fake )
{
  std_out << "# -- calibration calculation ----- " ;
  if( fake ) {
    std_out << " === FAKE ===";
  }
  std_out << NL;

  if( n_cal < 3 ) {
    std_out << "# Error: too few points: " << n_cal << NL;
    return false;
  }

  if( R_0_c > R_max  ||  R_0_c < 1e-3f ) {
    std_out << "# Error: calibrated R_0 is bad: " << R_0_c << NL;
    return false;
  }

  // find initial mode change
  unsigned i_lim = n_cal;
  float k_g = 0.12f;
  for( unsigned i=n_cal-1; i>0; --i ) {
    const float diff_pwm_l =  d_pwm[i]       - d_pwm[i-1];
    const float diff_pwm_g =  d_pwm[n_cal-1] - d_pwm[i-1];
    std_out << "# " << i << ' ';
    if( fabsf( diff_pwm_l ) < 0.1f || fabsf( diff_pwm_g ) < 0.1f ) {
      continue;
    }
    const float k_l1 = ( d_v[i]       - d_v[i-1] ) / diff_pwm_l;
    const float k_g1 = ( d_v[n_cal-1] - d_v[i-1] ) / diff_pwm_g;
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
  const float k_2 = - k_g * k_g / ( 4 * b );

  std_out << "# --- calibration check:" << NL;

  err_max = 0;
  for( unsigned i=0; i<n_cal; ++i ) {
    const float pwm = d_pwm[i];
    const float v = ( pwm > 2 * t_x_0 ) ? ( k_g * pwm + b ) : (  pwm * pwm * k_2 );
    const float err = fabsf( v - d_v[i] );
    if( err > err_max ) {
      err_max = err;
    }
    std_out << "# " << i << ' ' << pwm << ' ' << v << err << NL;
  }

  std_out << "# err_max= " << err_max << NL;
  std_out << "# R_0= " << R_0_c << " i_lim= " << i_lim << " k_gv1= " << k_g
          << " V_00= " << b << " x_0= " << t_x_0 << " k_gv2= " << k_2 << NL;

  if( err_max > 0.3f ) {
    std_out << "# Error: large approximation error!!! " << NL;
    return false;
  }

  R_0   = R_0_c;
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
  StatChannelXY ch_x;
  StatChannel   ch_y;

  for( unsigned i=0; i<max_cal_steps; ++i ) {
    // if( d_pwm[i] <= 0 ) { // next check is more strict
    //   continue;
    // }
    const bool good_point = d_pwm[i] > 2 * t_x0;
    const float x = d_pwm[i], y = d_v[i], w = d_wei[i];
    if( debug > 1 && x > 0 ) {
      std_out << "#*# " << i << ' ' << x << ' ' << y << ' ' << w << ' ' << good_point <<  NL;
    }
    if( !good_point ) {
      continue;
    }
    if( w < min_cal_req ) {
      continue;
    }
    ch_x.add( x, y );
    ch_y.add( y );
  }


  if( ch_x.n < 3 ) {
    std_out << "# warning: regre: n= " << ch_x.n << NL;
    return false;
  }

  RegreResults rr;
  if( ! regre( ch_x, ch_y, rr ) ) {
    std_out << "# Error: regre: = " << (int)rr.err << NL;
    return false;
  }

  if( debug > 1 ) {
    std_out << "# # regre: n= " << ch_x.n << NL
            << "# # sx = " << ch_x.sum << " sx2= " << ch_x.sum2
            << " sy= "     << ch_y.sum << " sy2= " << ch_y.sum2
            << " sxy= "    << ch_x.sum_xy << NL;
  }

  if( debug > 0 ) {
    std_out << "# # a = " << rr.a << " b= " << rr.b << " r= " << rr.r << NL;
  }

  if( rr.r < 0.7f ) {
    std_out << "# Warning: regre: r= " << r << NL;
    return false;
  }

  a = rr.a; b = rr.b; r = rr.r;
  need_regre = false;
  return true;
}

bool PWMInfo::doRegre()
{
  if( ! need_regre ) {
    return true;
  }
  float a = 0, b = 0, r = 0;
  bool ok = regreCalibration( x_0, a, b, r );

  // bad values by physics
  if( a < 1.0e-3f || a > 1.0e2f || b < -5.0f || b > 5.0f ) {
    ok = false;
  }

  if( debug > 0 || ! ok ) {
    std_out << "# doRegre1: a= " << a << " b= " << b << " r= " << r << " x_0= " << x_0 << " ok= " << ok << NL;
  }

  if( ok ) {
    k_gv1 = a; V_00 = b;
    fixCoeffs();
    if( debug > 0 ) {
      std_out << "# doRegre2: a= " << a << " b= " << b << " r= " << r << " x_0= " << x_0 << " ok= " << ok << NL;
    }
  }
  return ok;
}


bool PWMInfo::addSample( float pwm, float v )
{
  const auto i = cal_idx( pwm );
  if( i >= max_cal_steps ) {
    return false;
  }
  if( d_pwm[i] <= 0 ) { // first data in this bucket
    d_pwm[i] = pwm;
    d_v[i]   = v;
    d_wei[i] = 1;
  } else {
    d_pwm[i] = k_move * pwm + ( 1.0f - k_move ) * d_pwm[i];
    d_v[i]   = k_move * v   + ( 1.0f - k_move ) *   d_v[i];
    ++d_wei[i];
  }
  need_regre = true;
  return true;
}

void PWMInfo::printData( bool more ) const
{
  std_out << "# PWMInfo data" NL;
  std_out << "#  n_cal= " << n_cal
          << "  V_00= " << V_00
          << "  k_gv1= " << k_gv1
          << "  k_gv2= " << k_gv2
          << "  x_0= " << x_0
          << "  was_calibr= " << was_calibr
          << "  need_regre= " << need_regre
          << NL;
  unsigned n = more ? max_cal_steps : n_cal;
  for( unsigned i=0; i < n; ++i ) {
    std_out << "#  " << FmtInt( i, 2 ) << ' ' << d_pwm[i] << ' ' << d_v[i] << ' ' << d_wei[i] << NL;
  }
}

// ------------------------ PWMData -------------------------------------

void PWMData::reset_steps()
{
  for( auto &s : steps ) {
    s.setDefault( pwm_def );
  }
  n_steps = 0;
}

int PWMData::add_step( float b, float e, int ms, pwm_type tp )
{
  if( n_steps >= size(steps) ) {
    return 0;
  }

  steps[n_steps++] = { b, e, ms, tp };

  return n_steps;
}

void PWMData::mk_rect( float vmin, float vmax, int t, pwm_type tp )
{
  reset_steps();
  add_step( vmin, vmin, 10000, tp );
  add_step( vmax, vmax,     t, tp );
  add_step( vmin, vmin, 60000, tp );
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
  add_step( vmin, vmin, 10000, tp );
  add_step( vmin, vmax,    t1, tp );
  add_step( vmax, vmax,    t2, tp );
  add_step( vmax, vmin,    t3, tp );
  add_step( vmin, vmin, 60000, tp );
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

void PWMData::off_pwm()
{
  if( set_pwm_real ) {
    set_pwm_real( 0 );
  }
}

bool PWMData::prep( int a_t_step, bool fake, const float *d )
{
  reason  = 0;
  pwm_tmax = pwm_max;
  t_step = a_t_step; fake_run = fake;
  t = 0; t_mul = 1;  c_step = 0; hand = 0; last_R = d[didx_r];
  pwm_intgr = 0;
  calcNextStep();
  if( fake_run ) {
    return true;
  }
  pwm_r = val;
  return true;
}

bool PWMData::tick( const float *d )
{
  t += t_step * t_mul;
  last_R = d[didx_r];
  auto pwm_val_old = pwm_val;

  if( fake_run ) {
    return true;
  }

  auto rc = check_lim( d );

  if( rc == check_result::hard ) {
    reason = 1;
    return false;
  }

  float err = 0, c_ki = 0;
  bool only_pwm = false;

  if( t >= step_t ) { // next step
    t = 0;
    ++c_step;
    if( c_step >= n_steps ) {
      reason = 0; // end
      return false;
    }
    calcNextStep();
  } else {

    val_1 = val_0 + ks * t;
    val = val_1 + hand;
    switch( steps[c_step].tp ) {
      case pwm_type::pwm:   pwm_base = pwm_val = val;
                            only_pwm = true;
                            break;
      case pwm_type::volt:  err = d[didx_v] - val;
                            c_ki = 1;
                            break;
      case pwm_type::curr:  err = d[didx_i] - val;
                            c_ki = last_R;
                            break;
      case pwm_type::pwr:   err = d[didx_w] - val;
                            c_ki = 1.0f /  max( d[didx_i], 0.1f );
                            break;
      default:              reason = 11;
                            return false;
    };
  }

  const float d_err_dt = ( err - last_err ) / t_step;
  last_err = err;

  if( !only_pwm ) {
    if( pwminfo.pid_only > 0 ) {
      pwm_base  = - pwminfo.kp_v * err; // * c_ki ???;
    } // TODO: rehint if flag

    pwm_intgr -= c_ki * pwminfo.ki_v * t_step * err;
    pwm_val = pwm_base + pwm_intgr -  c_ki * pwminfo.kd_v * d_err_dt ;
  }

  if( rc != check_result::ok && pwm_val > pwm_val_old ) {
    pwm_val = pwm_val_old;
  }

  pwm_val = clamp( pwm_val, pwm_min, pwm_tmax ); // to no-overintegrate
  set_pwm();

  pwminfo.addSample( pwm_val_old, d[didx_v] );


  return true;
}

void PWMData::calcNextStep()
{
  const float old_val = val;
  val = val_0  = steps[c_step].vb;
  step_t = steps[c_step].t;
  ks = ( steps[c_step].ve - val_0 ) / step_t;
  const bool rehint = ( c_step == 0  )
    || ( fabsf( val_0 - old_val ) > pwminfo.rehint_lim )
    || ( steps[c_step].tp != steps[c_step-1].tp );

  if( pwminfo.pid_only > 0 ) {
    return;
  }

  if( pwminfo.regre_lev > 0 ) { // not only for rehint!
    pwminfo.doRegre();
  }

  switch( steps[c_step].tp ) {
    case pwm_type::pwm:   pwm_base = val; pwm_intgr = 0; break;
    case pwm_type::volt:  if( rehint ) {
                            pwm_base = pwminfo.hint_for_V( val );
                            pwm_intgr = 0;
                          }
                          break;
    case pwm_type::curr:  if( rehint ) {
                            pwm_base = pwminfo.hint_for_V( val * last_R );
                            pwm_intgr = 0;
                          }
                          break;
    case pwm_type::pwr:  if( rehint ) {
                            pwm_base = pwminfo.hint_for_V( sqrtf( val * last_R  ) );
                            pwm_intgr = 0;
                          }
                          break;
    default:              pwm_base = pwm_min; break; // fail-safe
  };
  std_out
     << "# @@@ " << t << ' ' << c_step << ' ' << old_val << ' ' << val
     << ' ' << pwm_base << ' ' << last_R << ' ' << rehint << ' '
     << pwminfo.k_gv1 << ' ' << pwminfo.V_00 << NL;
}

void PWMData::end_run()
{
  if( ! fake_run ) {
    pwm_val = 0; hand = 0; t = 0; c_step = 0;
    pwm_intgr = 0; pwm_base = 0;
    off_pwm();
  }
  pwm_tmax = pwm_max;
}

PWMData::check_result PWMData::check_lim( const float *d )
{
  if( d[didx_r] > pwminfo.R_max  ||  d[didx_r] < 0.001f ) {
    std_out << "# @#@ Error: limit hard R: " << d[didx_r] << NL;
    leds.set( BIT0 );
    return check_result::hard;
  }

  if( d[didx_v] > pwminfo.V_max ) {
    pwm_val *= 0.95f * pwminfo.V_max / d[didx_v];
    pwm_tmax = 0.99f * pwm_val;
    leds.set( BIT1 );
    std_out << "# @#@ limit V: " << d[didx_v] << " pwm_tmax=" << pwm_tmax << NL;
    return check_result::soft;
  }
  if( d[didx_i] > pwminfo.I_max ) {
    pwm_val *= 0.95f * pwminfo.I_max / d[didx_i];
    pwm_tmax = 0.99f * pwm_val;
    std_out << "# @#@ limit I: " << d[didx_i] << " pwm_tmax=" << pwm_tmax << NL;
    leds.set( BIT1 );
    return check_result::soft;
  }
  if( d[didx_w] > pwminfo.W_max ) {
    pwm_val *= 0.95f * pwminfo.W_max / d[didx_w];
    pwm_tmax = 0.99f * pwm_val;
    std_out << "# @#@ limit W: " << d[didx_w] << " pwm_tmax=" << pwm_tmax << NL;
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
  // TODO: limits(type), for all funcs
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
  float   v0   = arg2float_d( 1, argc, argv,     3, 0.1f,       90 );
  float   dv   = arg2float_d( 2, argc, argv,     5, 0.0f,       90 );
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
    std_out << "# Error: need more argumets" << NL;
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
CmdInfo CMDINFO_MK_LADDER  { "mk_ladder",  'L', cmd_mk_ladder,  " vmin dv t n_up tp - make ladder steps"  };
CmdInfo CMDINFO_MK_RAMP    { "mk_ramp",    'R', cmd_mk_ramp,    " vmin vmax t1  t2 t3  tp - make ramp steps"  };
CmdInfo CMDINFO_EDIT_STEP  { "edit_step",  'E', cmd_edit_step,  " vb ve t tp - edit given step"  };

