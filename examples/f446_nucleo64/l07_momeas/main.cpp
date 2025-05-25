#include <cstring>
#include <cerrno>

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
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, "n v0 dv - test "  };
int cmd_init( int argc, const char * const * argv );
CmdInfo CMDINFO_INIT { "init", '\0', cmd_init, " - init RD6006"  };
int cmd_writeReg( int argc, const char * const * argv );
CmdInfo CMDINFO_WRITEREG { "write_reg", 'W', cmd_writeReg, "reg val - write 1 reg"  };
int cmd_readRegs( int argc, const char * const * argv );
CmdInfo CMDINFO_READREGS { "read_regs", 'R', cmd_readRegs, "start n - read n regs"  };
int cmd_readReg( int argc, const char * const * argv );
CmdInfo CMDINFO_READREG { "read_reg", '\0', cmd_readReg, "i - read 1 reg"  };
int cmd_on( int argc, const char * const * argv );
CmdInfo CMDINFO_ON { "on", '\0', cmd_on, "- set ON"  };
int cmd_off( int argc, const char * const * argv );
CmdInfo CMDINFO_OFF { "off", '\0', cmd_off, "- set OFF"  };
int cmd_measure( int argc, const char * const * argv );
CmdInfo CMDINFO_MEASURE { "measure", 'M', cmd_measure, "- measure V,I"  };
int cmd_setV( int argc, const char * const * argv );
CmdInfo CMDINFO_SETV { "setV", 'V', cmd_setV, "mV [r] - set output voltage "  };
int cmd_setI( int argc, const char * const * argv );
CmdInfo CMDINFO_SETI { "setI", 'I', cmd_setI, "100uA  [r]- set output current "  };
int cmd_measF( int argc, const char * const * argv );
CmdInfo CMDINFO_MEASF { "seasF", 'F', cmd_measF, "- measure force "  };
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", '\0', cmd_pwm, "v - set PWM 0-1000 "  };
int cmd_freq( int argc, const char * const * argv );
CmdInfo CMDINFO_FREQ { "freq", '\0', cmd_freq, "f - set PWM freq "  };
int cmd_timinfo( int argc, const char * const * argv );
CmdInfo CMDINFO_TIMINFO { "timinfo", '\0', cmd_timinfo, " - info about timers"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_INIT,
  &CMDINFO_WRITEREG,
  &CMDINFO_READREGS,
  &CMDINFO_READREG,
  &CMDINFO_ON,
  &CMDINFO_OFF,
  &CMDINFO_MEASURE,
  &CMDINFO_SETV,
  &CMDINFO_SETI,
  &CMDINFO_MEASF,
  &CMDINFO_PWM,
  &CMDINFO_FREQ,
  &CMDINFO_TIMINFO,
  nullptr
};

void set_pwm_freq( uint32_t f );
void set_pwm_vs( uint32_t vs );

extern UART_HandleTypeDef huart_modbus;
MODBUS_RTU_server m_srv( &huart_modbus );
RD6006_Modbus rd( m_srv );

HX711 hx711( HX711_SCK_GPIO, HX711_SCK_PIN, HX711_DAT_GPIO, HX711_DAT_PIN );
// -0.032854652221894 5.06179479849053e-07
xfloat hx_a =  5.0617948e-07f;
xfloat hx_b =  -0.03399918f;
StatChannel st_f;
uint32_t measure_f( int n );

#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }
ADD_FOBJ( hx_a  );
ADD_FOBJ( hx_b  );

constexpr const NamedObj *const objs_info[] = {
  & ob_hx_a,
  & ob_hx_b,
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

  UVAR('t') = 5000; // settle before measure
  UVAR('l') =    1; // break measurement if CC mode
  UVAR('u') =    2; // default MODBUS unit addr
  UVAR('m') =   30; // default force measure count
  UVAR('n') =   20; // default main loop count
  UVAR('f') =  100; // base PWM frequency
  UVAR('g') =    0; // PWM frequency shift
  UVAR('s') = 12000; // PWM scale = max

  UVAR('e') = MX_MODBUS_UART_Init();

  hx711.initHW();

  MX_TIM_CNT_Init();
  HAL_TIM_Base_Start( &htim_cnt );
  MX_TIM_PWM_Init();
  set_pwm_freq( 100 );
  set_pwm_vs( 0 );
  HAL_TIM_PWM_Start( &htim_pwm, TIM_PWM_CHANNEL );


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
  std_out << "#     1            2          3       4     5     6         7      8        9    10  11 12" NL;
  std_out << "#   V_set        V_out      I_out    pwm  freq    F        nF     rpx      dt    cnt cc e" NL;

  uint32_t scale = rd.getScale();
  if( ! scale ) {
    std_out << "# Error: scale = 0. init? " NL;
    return 2;
  }

  break_flag = 0;
  auto freq = UVAR('f');
  auto pwm = pwm0;
  set_pwm_vs( pwm );
  auto v_set = v0;
  rd.setV( v_set );
  rd.on();
  hx711.read( HX711::HX711_mode::mode_A_x128 ); // to init for next measurement
  delay_ms_brk( t_step );

  for( int i=0; i<n && !break_flag; ++i ) {
    ReturnCode rc;
    if( dv != 0 ) {
      rc = rd.setV( v_set );
      if( rc != rcOk ) {
        std_out << "# setV error: " << rc << NL;
        break;
      }
    }
    if( UVAR('g') != 0 ) {
      set_pwm_freq( freq );
    }
    if( dpwm !=0 || UVAR('g') != 0 ) {
      set_pwm_vs( pwm );
    }

    if( delay_ms_brk( t_step ) ) { // settle
      break;
    }

    uint32_t t0 = GET_OS_TICK();
    TIM_CNT->CNT = 0;
    measure_f( UVAR('m') );
    uint32_t t1 = GET_OS_TICK() - t0;
    uint32_t cnt = TIM_CNT->CNT;

    rc = rd.readMain();
    if( rc != rcOk ) {
      std_out << "# MODBUS readMain error: " << rc << NL;
      break;
    }
    auto err = rd.get_Err();
    auto cc  = rd.get_CC();
    uint32_t V = rd.getV_mV();
    xfloat V_f = V * 1e-3f;
    uint32_t I = rd.getI_100uA();
    xfloat I_f = I * 1e-4f;

    std_out << (v_set*1e-3f) << ' ' << V_f  << ' ' << I_f << ' ' << FmtInt(pwm,4) << ' ' << FmtInt(freq,5)
      << st_f.mean << ' '  << FmtInt(st_f.n,3) << ' ' << (xfloat)(cnt)/t1 << ' ' << t1 << ' ' << FmtInt(cnt,5)
      << ' ' << cc << ' ' << err << NL;

    if( err || ( cc && UVAR('l') && v_set > 80 ) ) { // 80 is mear minial v/o fake CC
      break;
    }

    v_set += dv; pwm += dpwm; freq += UVAR('g');
  }

  break_flag = 0;
  set_pwm_vs( 0 );
  std_out << "# prepare to OFF: " ;
  delay_ms( 200 );
  std_out << rd.off() << NL;

  return 0;
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

int cmd_measure( int argc, const char * const * argv )
{
  uint32_t scale = rd.getScale();
  if( ! scale ) {
    std_out << "# measure Error: scale = 0 " NL;
    return 2;
  }
  auto err = rd.readErr();
  auto [v,i] = rd.read_VI();
  std_out << v << ' ' << i << ' ' << scale << ' ' << err << NL;
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
    auto [v,i] = rd.read_VI();
    std_out << v << ' ' << i << ' ' << err << NL;
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

  TickType t0 = GET_OS_TICK();
  measure_f( n );
  TickType t1 = GET_OS_TICK();

  std_out << "# force: " << st_f.mean << ' ' << st_f.n << ' ' << st_f.sd << ' ' << ( t1-t0 )<< NL;
  return 0;
}

uint32_t measure_f( int n )
{
  st_f.reset();
  for( int i=0; i<n; ++i ) {
    auto fo_ret = hx711.read( HX711::HX711_mode::mode_A_x128 );
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
  uint32_t f = arg2long_d( 1, argc, argv, 1000, 1, 200000 );
  set_pwm_freq( f );
  std_out << "# freq: " << f << NL;
  return 0;
}

void set_pwm_vs( uint32_t vs )
{
  uint32_t scale = ( UVAR('s') > 0 ) ? UVAR('s') : 1;
  auto ccr = uint32_t((uint64_t)TIM_PWM->ARR * vs / scale );
  TIM_PWM->PWM_CCR = ccr;
}

void set_pwm_freq( uint32_t f )
{
  auto old_arr = TIM_PWM->ARR;
  if( old_arr < 1 ) { old_arr = 1; }
  auto old_ccr = TIM_PWM->PWM_CCR;
  auto arr = calc_TIM_arr_for_base_freq( TIM_PWM, f );
  TIM_PWM->ARR = arr;
  TIM_PWM->PWM_CCR = uint32_t( (uint64_t)old_ccr * arr / old_arr );
  TIM_PWM->CNT = 0;
}


int cmd_timinfo( int argc, const char * const * argv )
{
  tim_print_cfg( TIM_CNT );
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

