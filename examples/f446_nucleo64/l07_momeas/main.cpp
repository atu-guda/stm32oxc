#include <cstring>
#include <cerrno>
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_hx711.h>
#include <oxc_statdata.h>
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
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", '\0', cmd_pwm, "v - set PWM 0-1000"  };
int cmd_freq( int argc, const char * const * argv );
CmdInfo CMDINFO_FREQ { "freq", '\0', cmd_freq, "f - set PWM freq"  };
int cmd_timinfo( int argc, const char * const * argv );
CmdInfo CMDINFO_TIMINFO { "timinfo", '\0', cmd_timinfo, " - info about timers"  };
int cmd_vstep( int argc, const char * const * argv );
CmdInfo CMDINFO_VSTEP { "vstep", 'Z', cmd_vstep, " - set set voltage and measure"  };

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
  &CMDINFO_PWM,
  &CMDINFO_FREQ,
  &CMDINFO_TIMINFO,
  &CMDINFO_VSTEP,
  nullptr
};

void set_pwm_freq( xfloat f );
void set_pwm_vs( uint32_t vs );
uint32_t calc_TIM_arr_for_base_freq_xfloat( TIM_TypeDef *tim, xfloat base_freq ); // like oxc_tim, but xfloat


extern UART_HandleTypeDef huart_modbus;
MODBUS_RTU_server m_srv( &huart_modbus );
RD6006_Modbus rd( m_srv );
ReturnCode do_off();

xfloat freq_min { 100.0f }, freq_max { 100.0f };

constexpr auto hx_mode = HX711::HX711_mode::mode_A_x128;
HX711 hx711( HX711_SCK_GPIO, HX711_SCK_PIN, HX711_DAT_GPIO, HX711_DAT_PIN );
// -0.032854652221894 5.06179479849053e-07
xfloat hx_a { 5.0617948e-07f };
xfloat hx_b {  -0.03399918f };
StatChannel st_f;
uint32_t measure_f( int n );

xfloat gears_r { 27.65f * 12 }; // gears*puls/turn ratio
//xfloat gears_r { 1.0f * 20 }; // for opto sensor

#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }
ADD_FOBJ( hx_a  );
ADD_FOBJ( hx_b  );
ADD_FOBJ( gears_r );
ADD_FOBJ( freq_min );
ADD_FOBJ( freq_max );

constexpr const NamedObj *const objs_info[] = {
  & ob_hx_a,
  & ob_hx_b,
  & ob_gears_r,
  & ob_freq_min,
  & ob_freq_max,
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
  UVAR('s') = 12000; // PWM scale = max
  UVAR('v') =  1000; // voltage step
  UVAR('w') =  4000; // current voltage in steps - 'v'
  UVAR('z') =     1; // drop low subzero force

  UVAR('e') = MX_MODBUS_UART_Init();

  hx711.initHW();

  MX_TIM_CNT_Init();
  MX_TIM_CNT2_Init();
  HAL_TIM_Base_Start( &htim_cnt );
  HAL_TIM_Base_Start( &htim_cnt2 );
  MX_TIM_PWM_Init();
  set_pwm_freq( freq_min );
  set_pwm_vs( 0 );
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
  int n    = arg2long_d( 1, argc, argv,  UVAR('n'), 0 );
  int v0   = arg2long_d( 2, argc, argv,  0, 0, 50000 );
  int dv   = arg2long_d( 3, argc, argv, 10, 0, 10000 );
  int pwm0 = arg2long_d( 4, argc, argv,  0, 0 );
  int dpwm = arg2long_d( 5, argc, argv,  0, 0, UVAR('s') );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step
          << " v0= " << v0 << " dv= " << dv << " pwm0= " << pwm0 << " dpwm= " << dpwm << " pwm_max= " << UVAR('s') << NL;

  auto out_v = [](xfloat x) { return FltFmt(x, cvtff_fix,8,4); };

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
  set_pwm_freq( freq );
  auto pwm = pwm0;
  set_pwm_vs( pwm );
  auto v_set = v0;
  rd.setV( v_set );
  rd.on();
  std_out << "# preheat: " << freq << ' ' << pwm << ' ' << v_set << NL;
  std_out << "#   1        2        3        4      5         6         7        8       9       10    11 12 13 14" NL;
  std_out << "# V_set    V_out    V_eff    I_out   pwm      freq        F        nF     rps      dt    cnt cc e i" NL;
  delay_ms_brk( t_step );

  for( int i=0; i<n && !break_flag; ++i ) {
    ReturnCode rc;
    leds[0] = 1;
    if( dv != 0 ) {
      rc = rd.setV( v_set );
      if( rc != rcOk ) {
        std_out << "# setV error: " << rc << NL;
        break;
      }
    }

    if( do_change_freq ) {
      freq = freq_min * expxf( i * k_freq );
      set_pwm_freq( freq );
    }
    if( dpwm !=0 || do_change_freq ) {
      set_pwm_vs( pwm );
    }
    leds[0] = 0;

    leds[1] = 1;
    if( delay_ms_brk( t_step ) ) { // settle
      break;
    }
    leds[1] = 0;

    uint32_t t0 = GET_OS_TICK();
    TIM_CNT->CNT  = 0;
    TIM_CNT2->CNT = 0;
    leds[2] = 1;
    measure_f( UVAR('m') );
    const uint32_t t1 = GET_OS_TICK() - t0;
    const uint32_t cnt = TIM_CNT->CNT; // TODO: CNT2 ?
    leds[2] = 0;

    rc = rd.readMain();
    if( rc != rcOk ) {
      std_out << "# MODBUS readMain error: " << rc << NL;
      break;
    }
    auto err = rd.get_Err();
    auto cc  = rd.get_CC();
    uint32_t V = rd.getV_mV();
    xfloat V_f = V * RD6006_V_scale;
    uint32_t I = rd.getI_100uA();
    xfloat I_f = I * RD6006_I_scale;
    xfloat V_eff = V_f * pwm / UVAR('s');
    xfloat force = st_f.mean;
    if( force < 0 && force > -1.0e-3f && UVAR('z') ) { // TODO: config
      force = 0;
    }

    std_out << out_v(v_set*RD6006_V_scale) << ' ' << out_v(V_f) << ' ' << out_v(V_eff) << ' ' << out_v(I_f) << ' '
      << FmtInt(pwm,5) << ' ' << freq << ' '
      << force << ' '  << FmtInt(st_f.n,3) << ' ' <<  (1e3f * cnt)/(t1*gears_r) << ' ' // 1e3f = systick/s
      << t1 << ' ' << FmtInt(cnt,5) << ' '
      << cc << ' ' << err << ' ' << i << NL;

    if( err || ( cc && UVAR('l') && v_set > 80 ) ) { // 80 is near minimal v/o fake CC
      leds[0] = 1;
      break;
    }

    v_set += dv;
    pwm += dpwm;
    pwm = std::clamp( pwm, 0, UVAR('s') );
  }

  break_flag = 0;
  std_out << "# prepare to OFF: " ;
  std_out << do_off() << NL;
  TIM_CNT->CNT = 0;
  TIM_CNT2->CNT = 0;

  return 0;
}

ReturnCode do_off()
{
  set_pwm_vs( 0 );
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

int cmd_measure_VI( int argc, const char * const * argv )
{
  uint32_t scale = rd.getScale();
  if( ! scale ) {
    std_out << "# measure Error: scale = 0 " NL;
    return 2;
  }
  auto err = rd.readErr();
  auto [V_m,I_m] = rd.read_VI();
  std_out << (V_m * RD6006_V_scale) << ' ' << (I_m * RD6006_I_scale) << ' ' << scale << ' ' << err << NL;
  return 0;
}

int cmd_setV( int argc, const char * const * argv )
{
  uint32_t V = arg2long_d( 1, argc, argv, 0, 0, 60000 );
  int     me = arg2long_d( 2, argc, argv, 0, 0, 1 );
  auto rc = rd.setV( V );
  std_out << "# setV " << V << ' ' << rc << NL;
  if( me ) {
    delay_ms( 1000 );
    auto err = rd.readErr();
    auto [V_m,I_m] = rd.read_VI();
    std_out << (V_m * RD6006_V_scale) << ' ' << (I_m * RD6006_I_scale) << ' ' << err << NL;
  }
  return 0;
}

int cmd_setI( int argc, const char * const * argv )
{
  uint32_t I = arg2long_d( 1, argc, argv, 0, 0, 50000 );
  auto rc = rd.setI( I );
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

  const TickType t0 = GET_OS_TICK();
  TIM_CNT->CNT  = 0;
  TIM_CNT2->CNT = 0;
  measure_f( n );
  const TickType t1 = GET_OS_TICK();
  const uint32_t cnt  = TIM_CNT->CNT;
  const uint32_t cnt2 = TIM_CNT2->CNT;

  if( off_after ) {
    do_off();
  }

  const auto dt = t1 - t0;
  const xfloat fr = ( dt != 0 ) ? ( 1.0e+3f * cnt / dt ) : 0;

  std_out << "# force: " << st_f.mean << ' ' << st_f.n << ' ' << st_f.sd << ' '
          << dt << ' ' << cnt << ' ' << cnt2 << ' ' << fr << NL;
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

int cmd_pwm( int argc, const char * const * argv )
{
  uint32_t pwm_s = arg2long_d( 1, argc, argv, 0, 0 );
  set_pwm_vs( pwm_s );
  std_out << "# pwm: " << pwm_s << ' ' << xfloat(pwm_s)/UVAR('s') << NL;
  return 0;
}

int cmd_freq( int argc, const char * const * argv )
{
  uint32_t f = arg2xfloat_d( 1, argc, argv, 100, 10, 200000 );
  set_pwm_freq( f );
  std_out << "# freq: " << f << NL;
  return 0;
}

int cmd_vstep( int argc, const char * const * argv )
{
  UVAR('w') += UVAR('v');
  const auto V = UVAR('w');
  if( V > 36000 ) {
    UVAR('w') = 4000;
    return 1;
  }
  rd.setV( V );
  delay_ms( 1000 );


  // measure ticks
  const TickType t0 = GET_OS_TICK();
  TIM_CNT ->CNT = 0;
  TIM_CNT2->CNT = 0;
  delay_ms( UVAR('t') );
  const TickType t1 = GET_OS_TICK();
  const uint32_t cnt  = TIM_CNT->CNT;
  const uint32_t cnt2 = TIM_CNT2->CNT;

  const auto dt = t1 - t0;
  const xfloat fr = ( dt != 0 ) ? ( 1.0e+3f * cnt / dt ) : 0;

  // measure consumption
  auto err = rd.readErr();
  auto [V_set,I_c] = rd.read_VI();

  std_out << ( V    * RD6006_V_scale) << ' '
          << (V_set * RD6006_V_scale) << ' '
          << (I_c   * RD6006_I_scale) << ' ' << err << ' '
          << dt << ' ' << cnt << ' ' << cnt2 <<  ' ' << fr << NL;

  return 0;
}


void set_pwm_vs( uint32_t vs )
{
  uint32_t scale = ( UVAR('s') > 0 ) ? UVAR('s') : 1;
  auto ccr = uint32_t((uint64_t)TIM_PWM->ARR * vs / scale );
  TIM_PWM->PWM_CCR = ccr;
}

void set_pwm_freq( xfloat f )
{
  auto old_arr = TIM_PWM->ARR;
  if( old_arr < 1 ) { old_arr = 1; }
  auto old_ccr = TIM_PWM->PWM_CCR;
  auto arr = calc_TIM_arr_for_base_freq_xfloat( TIM_PWM, f );
  TIM_PWM->ARR = arr;
  TIM_PWM->PWM_CCR = uint32_t( (uint64_t)old_ccr * arr / old_arr ); // approx old PWM
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

