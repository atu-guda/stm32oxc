#include <cstring>
#include <cerrno>
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_hx711.h>
#include <oxc_statdata.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
#include <oxc_modbus_rd6006.h>

#include "momeas.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure brushed motor params" NL;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, "n v0 dv pwm0 dpwm - test "  };
int cmd_init( int argc, const char * const * argv );
CmdInfo CMDINFO_INIT { "init", '\0', cmd_init, " - init RD6006"  };
int cmd_writeReg( int argc, const char * const * argv );
CmdInfo CMDINFO_WRITEREG { "write_reg", 'W', cmd_writeReg, "reg val - write 1 reg"  };
int cmd_readRegs( int argc, const char * const * argv );
CmdInfo CMDINFO_READREGS { "read_regs", 'R', cmd_readRegs, "start n - read n regs"  };
int cmd_readReg( int argc, const char * const * argv );
CmdInfo CMDINFO_READREG { "read_reg", '\0', cmd_readReg, "i - read 1 reg"  };
int cmd_on( int argc, const char * const * argv );
CmdInfo CMDINFO_ON { "on", '\0', cmd_on, "- set power ON"  };
int cmd_off( int argc, const char * const * argv );
CmdInfo CMDINFO_OFF { "off", '\0', cmd_off, "- set power OFF"  };
int cmd_measure_VI( int argc, const char * const * argv );
CmdInfo CMDINFO_MEASURE_VI { "measure_VI", 'M', cmd_measure_VI, "- measure V,I"  };
int cmd_setV( int argc, const char * const * argv );
CmdInfo CMDINFO_SETV { "setV", 'V', cmd_setV, "mV [r] - set output voltage"  };
int cmd_setI( int argc, const char * const * argv );
CmdInfo CMDINFO_SETI { "setI", 'I', cmd_setI, "100uA  [r]- set output current limit"  };
int cmd_measF( int argc, const char * const * argv );
CmdInfo CMDINFO_MEASF { "measF", 'F', cmd_measF, "[set_0] [off] - measure force"  };
int cmd_measure_Nu( int argc, const char * const * argv );
CmdInfo CMDINFO_MEASNU { "measNu", 'U', cmd_measure_Nu, "[dt] - measure rotation speed"  };
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", 'Q', cmd_pwm, "v - set PWM 0-pwm_max"  };
int cmd_freq( int argc, const char * const * argv );
CmdInfo CMDINFO_FREQ { "freq", '\0', cmd_freq, "f - set PWM freq"  };
int cmd_timinfo( int argc, const char * const * argv );
CmdInfo CMDINFO_TIMINFO { "timinfo", '\0', cmd_timinfo, " - info about timers"  };
int cmd_vstep( int argc, const char * const * argv );
CmdInfo CMDINFO_VSTEP { "vstep", 'Z', cmd_vstep, " - set+ voltage and measure"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_INIT,
  &CMDINFO_WRITEREG,
  &CMDINFO_READREGS,
  &CMDINFO_READREG,
  &CMDINFO_ON,
  &CMDINFO_OFF,
  &CMDINFO_MEASURE_VI,
  &CMDINFO_SETV,
  &CMDINFO_SETI,
  &CMDINFO_MEASF,
  &CMDINFO_MEASNU,
  &CMDINFO_PWM,
  &CMDINFO_FREQ,
  &CMDINFO_TIMINFO,
  &CMDINFO_VSTEP,
  nullptr
};

void set_pwm_freq( xfloat f, xfloat p );
void set_pwm( xfloat p );
uint32_t calc_TIM_arr_for_base_freq_xfloat( TIM_TypeDef *tim, xfloat base_freq ); // like oxc_tim, but xfloat


extern UART_HandleTypeDef huart_modbus;
MODBUS_RTU_server m_srv( &huart_modbus );
RD6006_Modbus rd( m_srv );
ReturnCode do_off();

xfloat freq_min { 100.0f }, freq_max { 100.0f };
auto out_v_fmt = [](xfloat x) { return FltFmt(x, cvtff_fix,8,4); };

constexpr auto hx_mode = HX711::HX711_mode::mode_A_x128;
HX711 hx711( HX711_SCK_GPIO, HX711_SCK_PIN, HX711_DAT_GPIO, HX711_DAT_PIN );
// -0.032854652221894 5.06179479849053e-07
xfloat hx_a { 5.0617948e-07f };
xfloat hx_b {  -0.03399918f };
StatChannel st_f;
uint32_t measure_f( int n );

int start_measure_times();
int stop_measure_times();
int measure_Nu( int dly );
void out_times( int se ); // bits: 1: start label, 2: NL

int measure_VIx();
void out_vi( int se );
ReturnCode set_V( xfloat v );

xfloat V_s { 0 };         // set voltage
xfloat V_m { 0 };         // measured voltage
xfloat I_m { 0 };         // measured current
xfloat V_max { 12.0f };   // max voltage - for fail-safe
xfloat V_step { 0.2f };   // default V step
xfloat V_eff  { 0.0f };   // current effective voltage
xfloat pwm_c { 0 };       // current pwm
xfloat t2p_1 { 11 }; // 11 (???) pulses/turn for Hall sensor
xfloat t2p_2 { 20 }; // 20       pulses/turn for Opto sensor
xfloat nu_1 { 0 };   // measured rps by Hall sensor
xfloat nu_2 { 0 };   // measured rps by Opto sensor
int    cnt_1  { 0 };  // first counter
int    cnt_2  { 0 };  // second counter
int    tick_0 { 0 };  // start tick
int    dlt_t    { 0 };   // ticks delta
int    v_err     { 0 }; // error during v seaturement
int    v_cc      { 0 }; // constant current state
int    pos_force { 1 }; // zero small nagative force

#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }
ADD_FOBJ( V_s  );
ADD_FOBJ( V_m  );
ADD_FOBJ( I_m  );
ADD_FOBJ( V_max );
ADD_FOBJ( V_step );
ADD_FOBJ( V_eff );
ADD_FOBJ( pwm_c );
ADD_FOBJ( hx_a  );
ADD_FOBJ( hx_b  );
ADD_FOBJ( t2p_1 );
ADD_FOBJ( t2p_2 );
ADD_FOBJ( nu_1 );
ADD_FOBJ( nu_2 );
ADD_FOBJ( freq_min );
ADD_FOBJ( freq_max );
ADD_IOBJ( cnt_1 );
ADD_IOBJ( cnt_2 );
ADD_IOBJ( tick_0 );
ADD_IOBJ( dlt_t );
ADD_IOBJ( v_err );
ADD_IOBJ( v_cc );
ADD_IOBJ( pos_force );

constexpr const NamedObj *const objs_info[] = {
  & ob_V_s,
  & ob_V_m,
  & ob_I_m,
  & ob_V_max,
  & ob_V_step,
  & ob_V_eff,
  & ob_pwm_c,
  & ob_hx_a,
  & ob_hx_b,
  & ob_t2p_1,
  & ob_t2p_2,
  & ob_nu_1,
  & ob_nu_2,
  & ob_freq_min,
  & ob_freq_max,
  & ob_cnt_1,
  & ob_cnt_2,
  & ob_tick_0,
  & ob_dlt_t,
  & ob_v_err,
  & ob_v_cc,
  & ob_pos_force,
  nullptr
};

NamedObjs objs( objs_info );

// print/set hook functions

bool print_var_ex( const char *nm, int fmt )
{
  return objs.print( nm, fmt );
}

bool set_var_ex( const char *nm, const char *s )
{
  auto ok =  objs.set( nm, s );
  print_var_ex( nm, 0 );
  return ok;
}

void idle_main_task()
{
  // leds.toggle( 1 );
}




int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 4000; // settle before measure
  UVAR('l') =    1; // break measurement if CC mode
  UVAR('u') =    2; // default MODBUS unit addr
  UVAR('m') =   40; // default force measure count
  UVAR('n') =   20; // default main loop count
  UVAR('v') =  1000; // voltage step
  UVAR('w') =  4000; // current voltage in steps - 'v'

  UVAR('e') = MX_MODBUS_UART_Init();

  hx711.initHW();

  MX_TIM_CNT_Init();
  MX_TIM_CNT2_Init();
  HAL_TIM_Base_Start( &htim_cnt );
  HAL_TIM_Base_Start( &htim_cnt2 );
  MX_TIM_PWM_Init();
  set_pwm_freq( freq_min, 0 );
  HAL_TIM_PWM_Start( &htim_pwm, TIM_PWM_CHANNEL );

  rd.setAddr( UVAR('u') ); // default
  rd.init();

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n        = arg2long_d(   1, argc, argv, UVAR('n'),  1, 10000 );
  xfloat v0    = arg2xfloat_d( 2, argc, argv,      0,     0, V_max );
  xfloat dv    = arg2xfloat_d( 3, argc, argv, V_step,     0, 10.0f );
  xfloat pwm0  = arg2xfloat_d( 4, argc, argv,      0,     0,  1.0f );
  xfloat dpwm  = arg2xfloat_d( 5, argc, argv,      0,     0,  1.0f );
  uint32_t t_step = UVAR('t');

  if( pwm0 + (n-1) * dpwm > 1.0f ) {
    dpwm = ( 1.0f - pwm0 ) / (n-1);
  }

  std_out <<  "# Test0: n= " << n << " t= " << t_step
          << " v0= " << v0 << " dv= " << dv << " pwm0= " << pwm0 << " dpwm= " << dpwm << NL;


  uint32_t scale = rd.getScale();
  if( ! scale ) {
    std_out << "# Error: scale = 0. init? " NL;
    return 2;
  }

  if( freq_min < 10 || freq_max < 10 ) {
    std_out << "# Error: freq too low: " << freq_min << ' ' << freq_max << NL;
    return 3;
  }
  xfloat k_freq { 0 };
  bool do_change_freq { false };
  if( n > 1 ) {
    k_freq = logxf( freq_max / freq_min ) / ( n - 1 );
  }
  if( fabsxf( k_freq ) > 1e-5f ) {
    do_change_freq = true;
  }

  hx711.read( hx_mode ); // to init for next measurement

  break_flag = 0;
  auto freq = freq_min;
  auto pwm = pwm0;
  set_pwm_freq( freq, pwm );
  auto v_set = v0;
  if( set_V( v_set ) != rcOk ) {
    std_out << "# Error: fail to set V" << NL;
    return 3;
  };
  rd.on();
  std_out << "# preheat: " << freq << ' ' << pwm << ' ' << v_set << NL;
  // TODO: fix order!
  std_out << "#   1        2        3        4      5         6         7        8       9       10    11 12 13 14" NL;
  std_out << "# V_set    V_out    V_eff    I_out   pwm      freq        F        nF     rps      dt    cnt cc e i" NL;
  delay_ms_brk( t_step );

  bool need_v_change   = fabsxf( dv )   > 1e-4f;
  bool need_pwm_change = fabsxf( dpwm ) > 1e-5f;

  for( int i=0; i<n && !break_flag; ++i ) {
    leds[0] = 1;
    v_set = std::clamp( v0   + dv   * i, 0.0f, V_max );
    pwm   = std::clamp( pwm0 + dpwm * i, 0.0f, 1.0f );

    if( need_v_change ) {
      auto rc = set_V( v_set );
      if( rc != rcOk ) {
        std_out << "# set_V error: " << rc << NL;
        break;
      }
    }

    if( do_change_freq ) {
      freq = freq_min * expxf( i * k_freq );
      set_pwm_freq( freq, pwm );
    } else if( need_pwm_change ) {
      set_pwm( pwm );
    }
    leds[0] = 0;

    leds[1] = 1;
    if( delay_ms_brk( t_step ) ) { // settle
      break;
    }
    leds[1] = 0;

    leds[2] = 1;
    start_measure_times();
    measure_f( UVAR('m') );
    measure_VIx();
    stop_measure_times();
    leds[2] = 0;

    if( v_err != 0 ) {
      std_out << "# MODBUS readMain error: " << v_err << NL;
      break;
    }

    xfloat force = st_f.mean;
    if( force < 0 && force > -1.0e-3f && pos_force ) { // TODO: more config
      force = 0;
    }

    out_vi( 0 );
    std_out << ' ' << out_v_fmt(V_eff) << ' ' << out_v_fmt(pwm) << ' ' << FmtInt(freq,6) << ' '
      << force << ' '  << FmtInt(st_f.n,3);
    out_times( 2 );

    if( v_err || ( v_cc && UVAR('l') && V_s > 0.08f ) ) { // 80 is near minimal v/o fake CC
      leds[0] = 1;
      break;
    }

  }

  break_flag = 0;
  std_out << "# prepare to OFF: " ;
  std_out << do_off() << NL;

  return 0;
}

ReturnCode do_off()
{
  set_pwm( 0 );
  delay_ms( 200 );
  return rd.off();
}

int cmd_init( int argc, const char * const * argv )
{
  uint8_t addr = arg2long_d( 1, argc, argv, UVAR('u'), 0, 0xFFFF );
  std_out <<  "#  init: addr=" << (int)addr  << NL;
  rd.setAddr( addr );
  auto rc = rd.init();
  std_out << "# scale= " << rd.getScale() << " rc= " << rc << NL;

  return 0;
}

int cmd_on( int argc, const char * const * argv )
{
  auto rc = rd.on();
  std_out << "# ON.  rc= " << rc << NL;
  return 0;
}

int cmd_off( int argc, const char * const * argv )
{
  auto rc = rd.off();
  std_out << "# OFF. rc= " << rc << NL;
  return 0;
}

ReturnCode set_V( xfloat v )
{
  uint32_t Vi = v / RD6006_V_scale;
  auto rc = rd.setV( Vi );
  if( rc == rcOk ) {
    V_s = v;
    V_eff = V_s * pwm_c;
  }
  return rc;
}


int measure_VIx()
{
  uint32_t scale = rd.getScale();
  if( ! scale ) {
    v_err = 255;
    return 0;
  }
  auto rc = rd.readMain();
  if( rc != rcOk ) {
    v_err = 254; //"# MODBUS readMain error: "
    return 0;
  }
  v_err = rd.get_Err();
  v_cc  = rd.get_CC();
  uint32_t Vi = rd.getV_mV();
  V_m = Vi * RD6006_V_scale;
  uint32_t Ii = rd.getI_100uA();
  I_m = Ii * RD6006_I_scale;
  V_eff = V_s * pwm_c;
  return 1;
}

int cmd_measure_VI( int argc, const char * const * argv )
{
  auto rc = measure_VIx();
  if( ! rc ) {
    std_out << "# measure Error: " << v_err << NL;
    return 2;
  }
  out_vi( 3 );
  return 0;
}

void out_vi( int se )
{
  if( se & 1 ) {
    std_out << "# VI: ";
  }
  std_out << ' ' << out_v_fmt(V_s)
          << ' ' << out_v_fmt(V_m)
          << ' ' << out_v_fmt(V_eff)
          << ' ' << out_v_fmt(I_m)
          << ' ' << v_cc << ' ' << v_err;
  if( se & 2 ) {
    std_out << NL;
  }
}

int cmd_setV( int argc, const char * const * argv )
{
  xfloat V = arg2xfloat_d( 1, argc, argv, 0, 0, 60.0f );
  int     me = arg2long_d( 2, argc, argv, 0, 0, 1 );
  auto Vi = V / RD6006_V_scale;
  auto rc = rd.setV( Vi );
  std_out << "# setV " << V << ' ' << rc << NL;
  V_s = V;
  if( me ) {
    delay_ms( 1000 );
    measure_VIx();
    out_vi( 3 );
  }
  return 0;
}

int cmd_setI( int argc, const char * const * argv )
{
  xfloat I = arg2xfloat_d( 1, argc, argv, 0, 0, 5.0f );
  auto Ii = I / RD6006_I_scale;
  auto rc = rd.setI( Ii );
  delay_ms( 100 );
  std_out << "# setI " << I << ' ' << rc << NL;
  return 0;
}

// ------------------------------------------------------------ 

int cmd_measF( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('m'), 1, 10000 );
  int set_zero  = arg2long_d( 2, argc, argv, 0, 0, 1 );
  int off_after = arg2long_d( 3, argc, argv, 0, 0, 1 );

  start_measure_times();
  measure_f( n );
  stop_measure_times();

  if( off_after ) {
    do_off();
  }

  std_out << "# force: " << st_f.mean << ' ' << st_f.n << ' ' << st_f.sd;
  out_times( 2 );
  if( set_zero ) {
    hx_b -= st_f.mean;
  }
  return 0;
}

uint32_t measure_f( int n )
{
  st_f.reset();
  hx711.read( hx_mode );
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    auto fo_ret = hx711.read( hx_mode );
    if( fo_ret.isOk() ) {
      auto fo_i = fo_ret.isOk() ? fo_ret.v : 0;
      xfloat fo = fo_i * hx_a + hx_b;
      st_f.add( fo );
    }
  }
  st_f.calc();
  return st_f.n;
}

int cmd_measure_Nu( int argc, const char * const * argv )
{
  int dly = arg2long_d( 1, argc, argv, 2000, 1, 100000 );

  measure_Nu( dly );
  out_times( 3 );
  return 0;
}

int start_measure_times()
{
  tick_0 = GET_OS_TICK();
  TIM_CNT->CNT  = 0; TIM_CNT2->CNT = 0;
  return tick_0;
}

int stop_measure_times()
{
  const TickType t1 = GET_OS_TICK();
  cnt_1 = TIM_CNT->CNT;
  cnt_2 = TIM_CNT2->CNT;

  const auto dt = t1 - tick_0;
  const xfloat den1 = dt * t2p_1;
  const xfloat den2 = dt * t2p_2;
  nu_1 = ( den1 != 0 ) ? ( 1.0e+3f * cnt_1 / den1 ) : 0;
  nu_2 = ( den2 != 0 ) ? ( 1.0e+3f * cnt_2 / den2 ) : 0;

  return dt;
}

void out_times( int se )
{
  if( se & 1 ) {
    std_out << "# times: ";
  };
  std_out << ' ' << dlt_t << ' ' << cnt_1  << ' ' << cnt_2
          << ' '  << nu_1 << ' '  << nu_2 << ' ';
  if( se & 2 ) {
    std_out << NL;
  };
}

int measure_Nu( int dly )
{
  start_measure_times();
  delay_ms_brk( dly );
  return stop_measure_times();
}



int cmd_pwm( int argc, const char * const * argv )
{
  xfloat pwm = arg2xfloat_d( 1, argc, argv, 0.0f, 0.0f, 1.0f );
  set_pwm( pwm );
  std_out << "# pwm: " << pwm << NL;
  return 0;
}

int cmd_freq( int argc, const char * const * argv )
{
  uint32_t f = arg2xfloat_d( 1, argc, argv, 100, 10, 200000 );
  set_pwm_freq( f, pwm_c );
  std_out << "# freq: " << f << NL;
  return 0;
}

int cmd_vstep( int argc, const char * const * argv )
{
  auto v_s = V_s + V_step;
  if( v_s >= V_max ) {
    v_s = 4.0f; // near dry-run
  }
  set_V( v_s );
  delay_ms( 1000 );

  measure_Nu( UVAR('t') );
  measure_VIx();

  out_vi( 1 );
  out_times( 2 );

  return 0;
}


void set_pwm( xfloat p )
{
  auto ccr = uint32_t( TIM_PWM->ARR * p );
  TIM_PWM->PWM_CCR = ccr;
  pwm_c = p;
  V_eff = V_s * pwm_c;
}

void set_pwm_freq( xfloat f, xfloat p )
{
  auto old_arr = TIM_PWM->ARR;
  if( old_arr < 1 ) { old_arr = 1; }
  auto arr = calc_TIM_arr_for_base_freq_xfloat( TIM_PWM, f );
  TIM_PWM->ARR = arr;
  set_pwm( p );
  TIM_PWM->CNT = 0;
}

uint32_t calc_TIM_arr_for_base_freq_xfloat( TIM_TypeDef *tim, xfloat base_freq )
{
  uint32_t freq = get_TIM_cnt_freq( tim ); // cnf_freq
  uint32_t arr = uint32_t( freq / base_freq - 1 );
  return arr;
}


int cmd_timinfo( int argc, const char * const * argv )
{
  tim_print_cfg( TIM_CNT );
  tim_print_cfg( TIM_CNT2 );
  tim_print_cfg( TIM_PWM );
  return 0;
}

// ------------------------------------------------------------ 

// keep for debug. TODO: move to lib

int cmd_writeReg( int argc, const char * const * argv )
{
  uint16_t reg = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );
  uint16_t val = arg2long_d( 2, argc, argv, 0, 0, 0xFFFF );

  std_out <<  "# write1reg :  " << reg << ' ' << val << ' ' << UVAR('u') << NL;
  auto rc = m_srv.writeReg( UVAR('u'), reg, val );
  std_out << "# rc " << rc << ' ' << m_srv.getError() << ' ' << m_srv.getReplError() << NL;
  return rc;
}

int cmd_readRegs( int argc, const char * const * argv )
{
  uint16_t start = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );
  uint16_t n     = arg2long_d( 2, argc, argv, 1, 1, 125 );

  std_out <<  "# readNRegs :  " << start << ' ' << n << ' ' << UVAR('u') << NL;
  auto rc = m_srv.readRegs( UVAR('u'), start, n );

  std_out << "# rc " << rc << ' ' << m_srv.getError() << ' ' << m_srv.getReplError() << NL;

  if( rc == rcOk ) {
    for( uint16_t i=start; i<start+n; ++i ) {
      auto v = m_srv.getReg( i );
      std_out << "# " << HexInt16(i) << ' ' << HexInt16(v) << ' ' << v << NL;
    }
  }
  return rc;
}


int cmd_readReg( int argc, const char * const * argv )
{
  uint16_t i = arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );

  std_out <<  "# readNReg :  " << i << UVAR('u') << NL;
  auto v = m_srv.readGetReg( UVAR('u'), i );

  if( v ) {
    std_out << "# v= "  << HexInt16(v.value()) << ' ' << v.value() << NL;
  } else {
    std_out << "# rc " << v.error() << ' ' << m_srv.getError() << ' ' << m_srv.getReplError() << NL;
  }

  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

