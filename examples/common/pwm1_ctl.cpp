/*
 *    Description:  common functions to control PWM on 1 ch
 *        Version:  1.0
 *        Created:  01/06/19 19:08:25
*/
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

#include <../examples/common/inc/pwm1_ctl.h>

using namespace std;


void PWMData::reset_steps()
{
  for( auto &s : steps ) {
    s.v = vdef; s.t = 30000; s.tp = 0;
  }
  n_steps = 3;
}

void PWMData::mk_rect( float v, int t )
{
  steps[0].v = vdef; steps[0].t = 10000; steps[0].tp = 0;
  steps[1].v = v;    steps[1].t = t;     steps[1].tp = 0;
  steps[2].v = vdef; steps[2].t = 30000; steps[2].tp = 0;
  n_steps = 3;
}

void PWMData::mk_ladder( float dv, int t, unsigned n_up )
{
  unsigned n_up_max = max_pwm_steps / 2 - 1;
  n_up = clamp( n_up, 1u, n_up_max );

  steps[0].v = vdef; steps[0].t = 10000; steps[0].tp = 0;
  unsigned i = 1;
  float cv = vdef;
  for( /* NOP */; i <= n_up; ++i ) {
    cv += dv;
    steps[i].v = cv;
    steps[i].t = t; steps[i].tp = 0;
  }
  for( /* NOP */; i < n_up*2; ++i ) {
    cv -= dv;
    steps[i].v = cv;
    steps[i].t = t; steps[i].tp = 0;
  }
  steps[i].v = vdef; steps[i].t = 60000; steps[0].tp = 0;
  n_steps = n_up * 2 + 1;
}

void PWMData::mk_trap( float v, int t1, int t2, int t3 )
{
  steps[0].v = vdef; steps[0].t = 10000; steps[0].tp = 0;
  steps[1].v = vdef; steps[1].t = t1;    steps[1].tp = 1;
  steps[2].v = v;    steps[2].t = t2;    steps[2].tp = 0;
  steps[3].v = v;    steps[3].t = t3;    steps[3].tp = 1;
  steps[4].v = vdef; steps[4].t = 30000; steps[4].tp = 0;
  n_steps = 5;
}

void PWMData::show_steps() const
{
  std_out << "# vmin= " << vmin << "  vdef= " << vdef << "  vmax= " << vmax << " n_steps= " << n_steps << NL;
  int tc = 0;
  for( unsigned i=0; i<n_steps; ++i ) {
    std_out << '[' << i << "] " << tc << ' ' << steps[i].t << ' ' << steps[i].v << ' ' << steps[i].tp << NL;
    tc += steps[i].t;
  }
  std_out << "# Total: " << tc << " ms" NL;
}

bool PWMData::edit_step( unsigned ns, float v, int t, int tp )
{
  if( ns >= max_pwm_steps ) {
    return false;
  }
  steps[ns].v = v; steps[ns].t = t; steps[ns].tp = tp;
  if( ns >= n_steps ) {
    n_steps = ns+1;
  }
  return true;
}

void PWMData::set_pwm()
{
  val_r = clamp( val, vmin, vmax );
  uint32_t scl = tim_h.Instance->ARR;
  using tim_ccr_t = decltype( tim_h.Instance->CCR1 );
  tim_ccr_t nv = val_r * scl / 100;
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
    val_r = val;
  }
}

bool PWMData::tick()
{
  if( ! fake_run ) {
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
    set_pwm();
  }
  t += t_step * t_mul;
  return true;
}

void PWMData:: calcNextStep()
{
  val_0  = steps[c_step].v; val = val_0;
  step_t = steps[c_step].t;
  ks = 0;
  if( steps[c_step].tp == 1 && step_t > 0 && c_step+1 < n_steps ) {
    ks = ( steps[c_step+1].v - val_0 ) / step_t;
  }
}

void PWMData::end_run()
{
  if( ! fake_run ) {
    val = vdef; hand = 0; t = 0; c_step = 0;
    set_pwm();
  }
}

void PWMData::set_v_manual( float v )
{
  val = v; hand = 0;
  set_pwm();
}

// --------------------------- Commands -------------------------------------

DCL_CMD_REG( show_steps, 'S', " - show PWM steps"  );
int cmd_show_steps( int /*argc*/, const char * const * /*argv*/ )
{
  pwmdat.show_steps();
  return 0;
}

DCL_CMD_REG( set_minmax,   0, " pwm_min pwm_max - set PWM limits"  );
int cmd_set_minmax( int argc, const char * const * argv )
{
  float vmin = arg2float_d( 1, argc, argv, pwmdat.get_min(),  0.001, 98.00 );
  float vmax = arg2float_d( 2, argc, argv, pwmdat.get_max(), vmin+1, 99.99 );
  pwmdat.set_min( vmin );
  pwmdat.set_max( vmax );
  std_out << "# vmin= " << pwmdat.get_min() << " vmax= " << pwmdat.get_max() << NL;
  return 0;
}

DCL_CMD_REG( mk_rect,      0, " v t - make rectangle steps"  );
int cmd_mk_rect( int argc, const char * const * argv )
{
  float v  = arg2float_d( 1, argc, argv,    35, 1,       98 );
  int   t  = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  std_out << "# rect: v= " << v << " t= " << t << NL;
  pwmdat.mk_rect( v, t );
  pwmdat.show_steps();
  return 0;
}

DCL_CMD_REG( mk_ladder,    0, " v t n_up - make ladder steps"  );
int cmd_mk_ladder( int argc, const char * const * argv )
{
  float    v  = arg2float_d( 1, argc, argv,     5, 1,       90 );
  unsigned t  = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  unsigned n  = arg2long_d(  3, argc, argv,    10, 1, PWMData::max_pwm_steps/2-2 );
  std_out << "# ladder: v= " << v << " t= " << t << " n= " << n << NL;
  pwmdat.mk_ladder( v, t, n );
  pwmdat.show_steps();
  return 0;
}

DCL_CMD_REG( mk_trap,      0, " v t1  t2 t3 - make trapzoid steps"  );
int cmd_mk_trap( int argc, const char * const * argv )
{
  float  v = arg2float_d( 1, argc, argv,    50, 0,       99 );
  int   t1 = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  int   t2 = arg2long_d(  3, argc, argv, 30000, 1, 10000000 );
  int   t3 = arg2long_d(  4, argc, argv, 30000, 1, 10000000 );
  std_out << "# trap: v= " << v << " t1= " << t1 << " t2= " << t2 << " t3= " << t3  << NL;
  pwmdat.mk_trap( v, t1, t2, t3 );
  pwmdat.show_steps();
  return 0;
}

DCL_CMD_REG( edit_step,  'E', " v t tp - edit given step"  );
int cmd_edit_step( int argc, const char * const * argv )
{
  if( argc < 3 ) {
    return 1;
  }
  unsigned j = arg2long_d(1, argc, argv,     0, 0, PWMData::max_pwm_steps-1 );
  float v  = arg2float_d( 2, argc, argv,    25, 0, 99.99 );
  int   t  = arg2long_d(  3, argc, argv, 30000, 1, 10000000 );
  int   tp = arg2long_d(  4, argc, argv,     0, 0, 1 );
  bool ok = pwmdat.edit_step( j, v, t, tp );
  pwmdat.show_steps();
  return ok ? 0 : 1;
}


