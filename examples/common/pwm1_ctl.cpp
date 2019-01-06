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

PWMData pwmdat;


void pwm_reset_steps( PWMData &d )
{
  for( auto &s : d.steps ) {
    s.v = d.vmin; s.t = 30000; s.tp = 0;
  }
  d.n_steps = 3;
}

void pwm_mk_rect( PWMData &d, float v, int t )
{
  d.steps[0].v = d.vmin; d.steps[0].t = 10000; d.steps[0].tp = 0;
  d.steps[1].v = v;      d.steps[1].t = t;     d.steps[1].tp = 0;
  d.steps[2].v = d.vmin; d.steps[2].t = 30000; d.steps[2].tp = 0;
  d.n_steps = 3;
}

void pwm_mk_ladder( PWMData &d,  float dv, int t, unsigned n_up )
{
  unsigned n_up_max = max_pwm_steps / 2 - 1;
  n_up = clamp( n_up, 1u, n_up_max );

  d.steps[0].v = d.vmin; d.steps[0].t = 10000; d.steps[0].tp = 0;
  unsigned i = 1;
  float cv = d.vmin;
  for( /* NOP */; i <= n_up; ++i ) {
    cv += dv;
    d.steps[i].v = cv;
    d.steps[i].t = t; d.steps[i].tp = 0;
  }
  for( /* NOP */; i < n_up*2; ++i ) {
    cv -= dv;
    d.steps[i].v = cv;
    d.steps[i].t = t; d.steps[i].tp = 0;
  }
  d.steps[i].v = d.vmin; d.steps[i].t = 60000; d.steps[0].tp = 0;
  d.n_steps = n_up * 2 + 1;
}

void pwm_mk_trap( PWMData &d, float v, int t1, int t2, int t3 )
{
  d.steps[0].v = d.vmin; d.steps[0].t = 10000; d.steps[0].tp = 0;
  d.steps[1].v = d.vmin; d.steps[1].t = t1;    d.steps[1].tp = 1;
  d.steps[2].v = v;      d.steps[2].t = t2;    d.steps[2].tp = 0;
  d.steps[3].v = v;      d.steps[3].t = t3;    d.steps[3].tp = 1;
  d.steps[4].v = d.vmin; d.steps[4].t = 30000; d.steps[4].tp = 0;
  d.n_steps = 5;
}

void pwm_show_steps( const PWMData &d )
{
  STDOUT_os;
  os << "# vmin= " << d.vmin << "  vmax= " << d.vmax << " n_steps= " << d.n_steps << NL;
  int tc = 0;
  for( unsigned i=0; i<d.n_steps; ++i ) {
    os << '[' << i << "] " << tc << ' ' << d.steps[i].t << ' ' << d.steps[i].v << ' ' << d.steps[i].tp << NL;
    tc += d.steps[i].t;
  }
  os << "# Total: " << tc << " ms" NL;
}

int cmd_show_steps( int /*argc*/, const char * const * /*argv*/ )
{
  pwm_show_steps( pwmdat );
  return 0;
}

int cmd_set_minmax( int argc, const char * const * argv )
{
  pwmdat.vmin = arg2float_d( 1, argc, argv, 10, 1, 98 );
  pwmdat.vmax = arg2float_d( 2, argc, argv, 50, pwmdat.vmin+1, 99 );
  return 0;
}

int cmd_mk_rect( int argc, const char * const * argv )
{
  float v  = arg2float_d( 1, argc, argv,    35, 1, 98 );
  int   t  = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  pwm_mk_rect( pwmdat, v, t );
  pwm_show_steps( pwmdat );
  return 0;
}

int cmd_mk_ladder( int argc, const char * const * argv )
{
  float v      = arg2float_d( 1, argc, argv,    5,  1, 90 );
  unsigned  t  = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  unsigned  n  = arg2long_d(  3, argc, argv,     8, 1, max_pwm_steps/2-2 );
  pwm_mk_ladder( pwmdat, v, t, n );
  pwm_show_steps( pwmdat );
  return 0;
}

int cmd_mk_trap( int argc, const char * const * argv )
{
  float v  = arg2float_d( 1, argc, argv,    30, 0,       98 );
  int   t1 = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  int   t2 = arg2long_d(  3, argc, argv, 30000, 1, 10000000 );
  int   t3 = arg2long_d(  4, argc, argv, 30000, 1, 10000000 );
  pwm_mk_trap( pwmdat, v, t1, t2, t3 );
  pwm_show_steps( pwmdat );
  return 0;
}

int cmd_edit_step( int argc, const char * const * argv )
{
  unsigned j = arg2long_d(1, argc, argv,     0, 0, max_pwm_steps-1 );
  float v  = arg2float_d( 2, argc, argv,    25, 0, 99 );
  int   t  = arg2long_d(  3, argc, argv, 30000, 1, 10000000 );
  int   tp = arg2long_d(  4, argc, argv,     0, 0, 1 );
  pwmdat.steps[j].v = v; pwmdat.steps[j].t = t; pwmdat.steps[j].tp = tp;
  if( j >= pwmdat.n_steps ) {
    pwmdat.n_steps = j+1;
  }
  pwm_show_steps( pwmdat );
  return 0;
}

CmdInfo CMDINFO_SET_MINMAX { "set_minmax", 0, cmd_set_minmax, " pwm_min pwm_max - set PWM limits"  };
CmdInfo CMDINFO_SHOW_STEPS { "show_steps", 'S', cmd_show_steps, " - show PWM steps"  };
CmdInfo CMDINFO_MK_RECT { "mk_rect", 0, cmd_mk_rect, " v t - make rectangle steps"  };
CmdInfo CMDINFO_MK_LADDER { "mk_ladder", 0, cmd_mk_ladder, " v t n_up - make ladder steps"  };
CmdInfo CMDINFO_MK_TRAP { "mk_trap", 0, cmd_mk_trap, " v t1  t2 t3 - make trapzoid steps"  };
CmdInfo CMDINFO_EDIT_STEP { "edit_step", 'E', cmd_edit_step, " v t tp - edit given step"  };

