#include <cerrno>
#include <climits>
#include <cmath>
#include <cstring>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_atleave.h>
#include <oxc_fs_cmd0.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
//#include <oxc_outstr.h>
//#include <oxc_hd44780_i2c.h>
//#include <oxc_menu4b.h>
// #include <oxc_ds3231.h>


#include <fatfs_sd_st.h>
#include <ff.h>

#include "main.h"

#define OUT std_out

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

const char* common_help_string = "Application coordinate device control" NL;

int debug { 1 };
volatile int tim_mov_tick { 0 }; // set in TIM6_callback

extern SD_HandleTypeDef hsd;
void MX_SDIO_SD_Init();
uint8_t sd_buf[512]; // one sector
HAL_SD_CardInfoTypeDef cardInfo;
FATFS fs;

PinsOut stepdir_e1( GpioE,  0, 2 );
PinsOut stepdir_e0( GpioE, 14, 2 );
PinsOut stepdir_x(  GpioE,  8, 2 );
PinsOut stepdir_y(  GpioE, 10, 2 );
PinsOut stepdir_z(  GpioE, 12, 2 );


PinOut en_motors( GpioC, 11 );

void motors_off() {
  en_motors = 1;
}

void motors_on()  {
  en_motors = 0;
}

PinsOut aux3(  GpioD, 7, 4 );

PinsIn x_e(  GpioD, 0, 2, GpioRegs::Pull::down );
PinsIn y_e(  GpioD, 3, 2, GpioRegs::Pull::down );
PinsIn z_e(  GpioD, 5, 2, GpioRegs::Pull::down );

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;

array pwm_tims { &htim3, &htim10, &htim11 };

int MX_TIM2_Init();
int MX_TIM3_Init();  // PMW0
int MX_TIM4_Init();
int MX_TIM6_Init();  // tick clock for move
int MX_TIM10_Init(); // PWM1
int MX_TIM11_Init(); // PWM2
void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle );
int MX_PWM_common_Init( unsigned idx );

void TIM6_callback();


// to common PWM timers init
const TIM_ClockConfigTypeDef def_pwm_CSC {
    .ClockSource    = TIM_CLOCKSOURCE_INTERNAL,
    .ClockPolarity  = TIM_ICPOLARITY_RISING,
    .ClockPrescaler = TIM_CLOCKPRESCALER_DIV1,
    .ClockFilter    = 0
};
const TIM_MasterConfigTypeDef def_pwm_MasterConfig {
  .MasterOutputTrigger = TIM_TRGO_RESET,
  .MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE
};
const TIM_OC_InitTypeDef def_pwm_ConfigOC {
  .OCMode       = TIM_OCMODE_PWM1, // TIM_OCMODE_FORCED_INACTIVE,
  .Pulse        = 0,
  .OCPolarity   = TIM_OCPOLARITY_HIGH,
  .OCNPolarity  = TIM_OCNPOLARITY_HIGH,
  .OCFastMode   = TIM_OCFAST_DISABLE,
  .OCIdleState  = TIM_OCIDLESTATE_RESET,
  .OCNIdleState = TIM_OCNIDLESTATE_RESET
};

//                                   TODO: auto IRQ N
const EXTI_init_info extis[] = {
  { GpioD,  0, GpioRegs::ExtiEv::down,   EXTI0_IRQn,    1,  0 }, // D0: Xe-
  { GpioD,  1, GpioRegs::ExtiEv::down,   EXTI1_IRQn,    1,  0 }, // D1: Xe+
  { GpioE,  2, GpioRegs::ExtiEv::updown, (IRQn_Type)0 /*EXTI2_IRQn*/, 1,  0 }, // E2: touch: dis now
  { GpioD,  3, GpioRegs::ExtiEv::down,   EXTI3_IRQn,    1,  0 }, // D3: Ye-
  { GpioD,  4, GpioRegs::ExtiEv::down,   EXTI4_IRQn,    1,  0 }, // D4: Ye+
  { GpioD,  5, GpioRegs::ExtiEv::down,   EXTI9_5_IRQn,  1,  0 }, // D5: Ze-
  { GpioD,  6, GpioRegs::ExtiEv::down,   EXTI9_5_IRQn,  1,  0 }, // D6: Ze+
  { GpioA, 99, GpioRegs::ExtiEv::down,   EXTI0_IRQn,   15,  0 }  // 99>15: END
};


StepMover s_movers[n_motors] = {
// tick2mm x  dir max_speed max_l es_find_l k_slow endstops motor
  {   800, 0,  0,    500,     150,  5.0f,     0.1f,    &x_e, &stepdir_x  }, // TODO: 500 increase after working acceleration
  {   800, 0,  0,    500,     300,  5.0f,     0.1f,    &y_e, &stepdir_y  },
  {   800, 0,  0,    300,     150,  4.0f,     0.1f,    &z_e, &stepdir_z  },
  {   100, 0,  0,    100,  999999,  0.0f,     0.1f, nullptr, &stepdir_e0 },
  {   100, 0,  0,    100,  999999,  0.0f,     0.1f, nullptr, &stepdir_e1 }
};



Machine me_st( s_movers, std::size(s_movers) );


// C6  = T3.1  = PWM0
// B9  = T11.1 = PWM1
// B8  = T10.1 = PWM2
// B10 = T2.3  = PWM3?
// B11 = T2.4  = PWM4?
//
// A0:A4, ?A7 - ADC

// I2C_HandleTypeDef i2ch;
// DevI2C i2cd( &i2ch, 0 );
// HD44780_i2c lcdt( i2cd, 0x3F );
// HD44780_i2c *p_lcdt = &lcdt;
// DS3231 rtc( i2cd );


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " axis N [dt] - test"  };
int cmd_relmove( int argc, const char * const * argv );
CmdInfo CMDINFO_RELMOVE { "rel", 'R', cmd_relmove, "dx dy dz [feed] - rel move"  };
int cmd_absmove( int argc, const char * const * argv );
CmdInfo CMDINFO_ABSMOVE { "abs", 'A', cmd_absmove, "x y z [feed] - abs move"  };
int cmd_home( int argc, const char * const * argv );
CmdInfo CMDINFO_HOME { "home", 'H', cmd_home, "axis - go home at give axis"  };
int cmd_pwr( int argc, const char * const * argv );
CmdInfo CMDINFO_PWR { "pwr", 'P', cmd_pwr, "ch pow_f  - test PWM power control"  };
int cmd_gexec( int argc, const char * const * argv );
CmdInfo CMDINFO_GEXEC { "gexec", 'X', cmd_gexec, " file  - execute gcode file"  };
int cmd_fire( int argc, const char * const * argv );
CmdInfo CMDINFO_FIRE { "FIRE", 'F', cmd_fire, " power% time_ms  - fire laser"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  // DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  FS_CMDS0,
  &CMDINFO_RELMOVE,
  &CMDINFO_ABSMOVE,
  &CMDINFO_HOME,
  &CMDINFO_PWR,
  &CMDINFO_GEXEC,
  &CMDINFO_FIRE,
  nullptr
};

const constinit NamedInt   ob_break_flag     {    "break_flag",    const_cast<int*>(&break_flag)  }; // may be UB?
const constinit NamedInt   ob_debug          {    "debug",          &debug  };
const constinit NamedInt   ob_me_dly_xsteps  {    "me.dly_xsteps",  &me_st.dly_xsteps  };
// const constinit NamedInt   ob_me_dly_xsteps  {    "me.dly_xsteps",  [](){ me_st.get_dly_xsteps();}, [](xfloat v){ me_st.set_dly_xsteps(v);}  };
const constinit NamedFloat ob_me_fe_g0       {    "me.fe_g0",       &me_st.fe_g0  };
const constinit NamedFloat ob_me_fe_g1       {    "me.fe_g1",       &me_st.fe_g1  };
const constinit NamedFloat ob_me_fe_scale    {    "me.fe_scale",    &me_st.fe_scale  };
const constinit NamedFloat ob_me_spin100     {    "me.spin100",     &me_st.spin100  };
// const constinit NamedFloat ob_me_x           {    "me.x",           &me_st.x[0]  };
// const constinit NamedFloat ob_me_y           {    "me.y",           &me_st.x[1]  };
// const constinit NamedFloat ob_me_z           {    "me.z",           &me_st.x[2]  };
// const constinit NamedFloat ob_me_e0          {    "me.e0",          &me_st.x[3]  };
// const constinit NamedFloat ob_me_e1          {    "me.e1",          &me_st.x[4]  };
// const constinit NamedFloat ob_ma_x_es_find_l {    "ma.x.es_find",   &s_movers[0].es_find_l  };
// const constinit NamedFloat ob_ma_y_es_find_l {    "ma.y.es_find",   &s_movers[1].es_find_l  };
// const constinit NamedFloat ob_ma_z_es_find_l {    "ma.y.es_find",   &s_movers[2].es_find_l  };
// const constinit NamedFloat ob_ma_x_k_slow    {    "ma.x.k_slow",    &s_movers[0].k_slow  };
// const constinit NamedFloat ob_ma_y_k_slow    {    "ma.y.k_slow",    &s_movers[1].k_slow  };
// const constinit NamedFloat ob_ma_z_k_slow    {    "ma.z.k_slow",    &s_movers[2].k_slow  };

const constinit NamedObj *const objs_info[] = {
  & ob_break_flag,
  & ob_debug,
  & ob_me_dly_xsteps,
  & ob_me_fe_g0,
  & ob_me_fe_g1,
  & ob_me_fe_scale,
  & ob_me_spin100,
  // & ob_me_x,
  // & ob_me_y,
  // & ob_me_z,
  // & ob_me_e0,
  // & ob_me_e1,
  // & ob_ma_x_es_find_l,
  // & ob_ma_y_es_find_l,
  // & ob_ma_z_es_find_l,
  // & ob_ma_x_k_slow,
  // & ob_ma_y_k_slow,
  // & ob_ma_z_k_slow,
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

const char* endstops2str( uint16_t es, bool touch, char *buf )
{
  static char s_buf[8] { "XYZT??" }; // only 5 need
  if( !buf ) {
    buf = s_buf;
  }
  // pin-dependent code
  buf[0] = endstop2char( es ); // & 0x03 inside endstop2char fun
  buf[1] = endstop2char( es >> 3 ); // skip D2: sdio
  buf[2] = endstop2char( es >> 5 );
  buf[3] = touch ? '!' : ',';
  buf[4] = '\0';
  return buf;
}

const char* endstops2str_a( char *buf )
{
  auto es = endstops_gpio.IDR & endstops_mask;
  bool touch = touch_gpio.IDR & touch_mask;
  return endstops2str( es, touch, buf );
}

// handle Gnnn...., Mnnn..., !SOME_OTHER_COMMAND
int gcode_cmdline_handler( char *s )
{
  if( !s ) {
    return -1;
  };
  if( s[0] != '!' && s[0] != 'G' && s[0] != 'M' ) { // not my
    return -1;
  };
  // TODO: 'G' or 'M' only

  const char *cmd = s;
  if( cmd[0] == '!' ) { // skip '!'
    ++cmd;
  }
  std_out << NL "# gcode: cmd= \"" << cmd << '"' << NL;
  delay_ms( 10 );

  // list
  if( cmd[0] == 'G' && cmd[1] == '\0' ) {
    me_st.out_mg( false );
    return 0;
  }
  if( cmd[0] == 'M' && cmd[1] == '\0' ) {
    me_st.out_mg( true );
    return 0;
  }

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();

  GcodeBlock cb ( gcode_act_fun_me_st );
  int rc = cb.process( cmd );
  std_out << "# rc " << rc << " line \"" << cmd << "\" pos " << cb.get_err_pos() << NL;

  return 0;
}

int gcode_act_fun_me_st( const GcodeBlock &gc )
{
  return me_st.call_mg( gc );
}

void idle_main_task()
{
  auto e = GpioD.IDR & endstops_mask;
  leds[1] = ( e == endstops_mask ) ? 0 : 1;
}

int on_delay_actions()
{
  // leds.toggle( 1 );
  return 0;
}

// ---------------------------------------- main -----------------------------------------------

int main()
{
  STD_PROLOG_UART;

  UVAR('a') =         1; // Y axis
  UVAR('f') =       240; // mm/min = 4mm/s default speed
  UVAR('t') =         1;
  UVAR('n') =      1000;
  UVAR('s') =         6;
  UVAR('u') =       100;

  GpioA.enableClk(); GpioB.enableClk(); GpioC.enableClk(); GpioD.enableClk(); GpioE.enableClk();

  for( auto &m : s_movers ) {
    if( m.endstops != nullptr ) {
      m.endstops->initHW();
    }
    if( m.motor != nullptr ) {
      m.motor->initHW(); m.motor->write( 0 );
    }
  }

  aux3.initHW(); aux3 = 0;
  en_motors.initHW();
  motors_off();

  UVAR('e') = EXTI_inits( extis, true );

  MX_TIM3_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  if( ! MX_TIM6_Init() ) {
    die4led( 0x02 );
  }

  // UVAR('e') = i2c_default_init( i2ch );
  // i2c_dbg = &i2cd;
  // i2c_client_def = &lcdt;
  // lcdt.init_4b();
  // lcdt.cls();
  // lcdt.puts("I ");

  MX_SDIO_SD_Init();
  UVAR('u') = HAL_SD_Init( &hsd );
  delay_ms( 10 );
  UVAR('s') = HAL_SD_GetState( &hsd );
  UVAR('z') = HAL_SD_GetCardInfo( &hsd, &cardInfo );
  MX_FATFS_SD_Init();
  fs.fs_type = 0; // none
  fspath[0] = '\0';

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  cmdline_handlers[0] = gcode_cmdline_handler;
  cmdline_handlers[1] = nullptr;

  // just for now
  me_st.set_mode ( Machine::modeLaser );

  BOARD_POST_INIT_BLINK;

  leds.reset( 0xFF );


  pr( NL "##################### " PROJ_NAME NL );

  HAL_TIM_Base_Start_IT( &htim6 );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}



int cmd_test0( int argc, const char * const * argv )
{
  int a = arg2long_d( 1, argc, argv, UVAR('a'), 0, 100000000 ); // motor index
  int n = arg2long_d( 2, argc, argv, UVAR('n'), -10000000, 100000000 ); // number of pulses with sign
  uint32_t dt = arg2long_d( 3, argc, argv, UVAR('t'), 0, 1000 ); // ticks in ms

  bool rev = false;
  if( n < 0 ) {
    n = -n; rev = true;
  }

  if( (size_t)a > n_motors  ||  a < 0 ) {
    std_out << "# Error: bad motor index " << a << NL;
    return 2;
  }

  auto motor = s_movers[a].motor;
  if( ! motor ) {
    std_out << "# Error: motor not defined for " << a << NL;
    return 2;
  }

  motor->sr( 0x02, rev );
  motors_on();
  uint32_t tm0 = HAL_GetTick(), tc0 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    // uint32_t tmc = HAL_GetTick();
    // std_out << i << ' ' << ( tmc - tm0 )  << NL;

    leds[0].toggle();
    (*motor)[0].toggle();

    if( dt > 0 ) {
      delay_ms_until_brk( &tc0, dt );
    } else {
      delay_mcs( UVAR('u') );
    }

  }
  motors_off();

  int rc = break_flag;

  return rc + rev;
}

bool is_endstop_clear_for_dir( uint16_t e, int dir )
{
  if( dir == 0 || ( e & 0x03 ) == 0x03 ) { // no move or all clear
    return true;
  }
  if( dir >  0 && ( e & 0x02 ) ) { // forward and ep+ clear
    return true;
  }
  if( dir <  0 && ( e & 0x01 ) ) { // backward and ep- clear
    return true;
  }
  return false;
}


// TODO: move to Machine, simultanious movement
int go_home( unsigned axis )
{
  if( axis >= n_motors ) {
    std_out << "# Error: bad axis index " << axis << NL;
    return 0;
  }
  auto estp = s_movers[axis].endstops;
  if( s_movers[axis].max_l > 20000 || estp == nullptr ) {
    std_out << "# Error: unsupported axis  " << axis << NL;
    return 0;
  }

  pwm_off_all();
  // me_st.was_set = false;

  xfloat fe_slow = (xfloat)s_movers[axis].max_speed * s_movers[axis].k_slow;
  xfloat fe_fast = (xfloat)s_movers[axis].max_speed * 0.6f; // TODO: param too?

  const unsigned n_mo { 3 }; // 3 = only XYZ motors
  xfloat d_mm[n_mo];
  ranges::fill( d_mm, 0 );

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();

  // TODO: params
  int rc { 0 };

  // if on endstop_minus - go+
  auto esv = estp->read();
  std_out << "# debug: home: axis= " << axis << ' ' << endstops2str_a() << NL;
  if( is_endstop_minus_stop( esv ) ) {
    if( debug > 0 ) {
      std_out << "# Warning: move from endstop- " << axis << ' ' << endstops2str_a() << NL;
    }
    d_mm[axis] = s_movers[axis].es_find_l;
    rc = me_st.move_line( d_mm, n_mo, 2 * fe_slow );
    if( debug > 0 ) {
      std_out << "# rc= " << rc << NL;
    }
    // TODO: exit if bad?
  }

  esv = estp->read();
  if( ! is_endstop_clear( esv ) ) {
    std_out << "# Error: not all clear " << axis << ' ' << endstops2str_a() << NL;
    return 0;
  }

  // go to endstop
  d_mm[axis] = -2.0f * (xfloat)s_movers[axis].max_l; // far away
  if( debug > 0 ) {
    std_out << "# to_endstop ph 1 " << endstops2str_a() << NL;
  }
  rc = me_st.move_line( d_mm, n_mo, fe_fast );
  esv = estp->read();
  if( is_endstop_minus_go( esv ) ) { //
    std_out << "# Error: fail to find endstop. axis " << axis << " ph 1 " << esv  << ' ' << endstops2str_a() << NL;
    return 0;
  }

  // go slowly away
  if( debug > 0 ) {
    std_out << "# go_away ph 2 "  << endstops2str_a() << NL;
  }
  d_mm[axis] = s_movers[axis].es_find_l;
  rc = me_st.move_line( d_mm, n_mo, fe_slow, axis );
  esv = estp->read();
  if( is_endstop_any_stop(esv) ) { // must be ok
    std_out << "# Error: fail to step from endstop axis " << axis << " ph 2 " << endstops2str_a() << NL;
    return 0;
  }

  // go slowly to endstop
  if( debug > 0 ) {
    std_out << "# to_es ph 3 " << endstops2str_a() << NL;
  }
  d_mm[axis] = - 1.5f * s_movers[axis].es_find_l;
  rc = me_st.move_line( d_mm, n_mo, fe_slow );
  esv = estp->read();
  if( is_endstop_minus_go( esv ) ) {
    std_out << "# Error: fail to find endstop axis " << axis << " ph 3 " << endstops2str_a() << esv << NL;
    return 0;
  }

  // go slowly away again
  if( debug > 0 ) {
    std_out << "# go_away st ph 4 " << endstops2str_a() << NL;
  }
  d_mm[axis] = s_movers[axis].es_find_l;
  rc = me_st.move_line( d_mm, n_mo, fe_slow, axis );
  esv = estp->read();
  if( is_endstop_any_stop( esv ) ) {
    std_out << "# Error: fail to find endstop axis " << axis << " ph 4 " << endstops2str_a() << NL;
    return 0;
  }
  me_st.was_set = true;
  me_st.set_xn( axis, 0 );    // TODO: may be no auto?
  std_out << "# Ok: found endstop end  "  << endstops2str_a() << NL;

  return 1;
}

int cmd_relmove( int argc, const char * const * argv )
{
  std_out << "# relmove: " << NL;
  const unsigned n_mo { 3 }; // 3 = only XYZ motors

  xfloat d_mm[n_mo];

  for( unsigned i=0; i<n_mo; ++i ) {
    d_mm[i] = arg2xfloat_d( i+1, argc, argv, 0, -(xfloat)s_movers[i].max_l, (xfloat)s_movers[i].max_l );
  }
  xfloat fe_mmm = arg2xfloat_d( 4, argc, argv, UVAR('f'), 0.0f, 900.0f );

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();
  int rc = me_st.move_line( d_mm, n_mo, fe_mmm );

  return rc;
}

int cmd_absmove( int argc, const char * const * argv )
{
  std_out << "# absmove: " << NL;

  const unsigned n_mo { 3 }; // 3 = only XYZ motors

  xfloat d_mm[n_mo];

  for( unsigned i=0; i<n_mo; ++i ) {
    d_mm[i] = arg2xfloat_d( i+1, argc, argv, 0, -(xfloat)s_movers[i].max_l, (xfloat)s_movers[i].max_l )
            - me_st.get_xn( i );
  }
  xfloat fe_mmm = arg2xfloat_d( 4, argc, argv, UVAR('f'), 0.0f, 900.0f );

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();
  int rc = me_st.move_line( d_mm, n_mo, fe_mmm );

  return rc;
}



int cmd_home( int argc, const char * const * argv )
{
  unsigned axis = arg2long_d( 1, argc, argv, 0, 0, n_motors );

  std_out << "# home: " << axis << NL;

  int rc = go_home( axis );

  return rc;
}


int cmd_gexec( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    std_out << "# gexec error: need filename" << NL;
    return 1;
  }

  int max_lines  = arg2long_d( 2, argc, argv, INT_MAX-1, 0, INT_MAX   );
  int skip_lines = arg2long_d( 3, argc, argv,         0, 0, INT_MAX-2 );

  const char *fn = argv[1];

  std_out << "# exec gcode file \"" << fn << "\" max_lines= " << max_lines
    << " skip_lines= " << skip_lines << NL;

  FIL f;
  FRESULT r = f_open( &f, fn, FA_READ );
  if( r != FR_OK ) {
    std_out << "# gexec error: fail to open file \"" << fn << "\" r= " << r << NL;
    return 2;
  }

  const unsigned s_max { 256 };
  char s[s_max];
  int nl { 0 }, nle {0}, rsz {0}; // nle - executed lines

  DoAtLeave do_off_all( []() {  pwm_off_all(); motors_off(); } );
  motors_on();

  while( nle < max_lines ) {
    if( break_flag || !f_gets( s, s_max, &f ) ) {
      break;
    }
    auto l = strlen( s );
    rsz += l+1;
    if( s[l-1] == '\x0A' || s[l-1] == '\x0D' ) {
      s[l-1] = '\0';
    }
    std_out << NL "# \"" << s << "\" " << nl << ' ' << rsz << NL;

    if( nl < skip_lines ) {
      ++nl;
      std_out << "# skipped " NL;
      continue;
    }


    GcodeBlock cb ( gcode_act_fun_me_st );
    int rc = cb.process( s );
    ++nle;
    std_out << "# rc= " << rc << " br= " << break_flag << " nle= " << nle << NL;
    if( break_flag == 2 ) {
      break_flag = 0;
    }

    if( me_st.dly_xsteps > 0 ) { // TODO: move to machine! (and params)
      motors_off(); // TODO: investigate! + sensors
      delay_ms( me_st.dly_xsteps );
      motors_on();
    }

    if( rc >= GcodeBlock::rcEnd ) {
      std_out << " line \"" << s << "\" pos " << cb.get_err_pos() << ' ' << cb.get_err_code() << NL;
      break;
    }
    std_out <<  NL;
    ++nl;
  }

  f_close( &f );

  return r;
}

int cmd_fire( int argc, const char * const * argv )
{
  xfloat pwr = arg2xfloat_d( 1, argc, argv, 0.05, 0.0f, 100.0f );
  unsigned dt = arg2long_d( 2, argc, argv, 0, 0, 10000 );
  std_out << "# fire: pwr= " << pwr << " dt=  " << dt << NL;

  pwm_set( 0, pwr );
  delay_ms_brk( dt );
  pwm_set( 0, 0 );

  return 0;
}

// -------------------- MoveInfo ------------------------------

MoveInfo::MoveInfo( MoveInfo::Type tp, unsigned a_n_coo, Act_Pfun pfun  )
  : type( tp ), n_coo( a_n_coo ), step_pfun( pfun )
{
}

void MoveInfo::zero_arr()
{
  ranges::fill( p, 0 ); ranges::fill( ci, 0 ); ranges::fill( k_x, 0 );
}

int MoveInfo::prep_move_line( const xfloat *coo, xfloat fe )
{
  zero_arr(); len = 0;

  for( unsigned i=0; i<n_coo; ++i ) {
    p[i] = coo[i]; len += coo[i] * coo[i];
  }
  len = sqrtxf( len );
  t_sec = 60 * len / fe; // only approx, as we can accel/deccel
  t_tick = (uint32_t)( TIM6_count_freq * t_sec );

  for( unsigned i=0; i<n_coo; ++i ) {
    k_x[i] = p[i];
  }
  OUT << "# debug: prep_move_line: len= " << len << " t_sec= " << t_sec << " t_tick= " << t_tick << NL;
  return 0;
}

int MoveInfo::prep_move_circ( const xfloat *coo, xfloat fe )
{
  zero_arr();
  //                       r_s      r_e                alp_e    alp_s
  xfloat l_appr { 0.5f * ( coo[0] + coo[2] ) * fabsxf( coo[3] - coo[1] ) };
  len = hypot( l_appr, coo[4] ); // z_e
  t_sec = 60 * len / fe; // only approx, as we can accel/deccel
  t_tick = (uint32_t)( TIM6_count_freq * t_sec );
  for( unsigned i=0; i<max_params; ++i ) { // 10 is N params to circle funcs
    p[i] = coo[i];
  }

  k_x[0] = p[3] - p[1]; // alp_e - alp_s
  k_x[1] = p[2] - p[0]; // r_e   - r_s

  // initial point is not (0,0) TODO: make (0,) -- better calc coords
  ci[0] = ( p[8] + p[0] * cos( p[1] )  ) * s_movers[0].tick2mm * me_st.axis_scale[0]; // TODO: remove me_st from here?!!
  ci[1] = ( p[8] + p[0] * sin( p[1] )  ) * s_movers[1].tick2mm * me_st.axis_scale[1]; // TODO: remove me_st from here?!!

  OUT << "# debug: prep_move_circ: len= " << len << " t_sec= " << t_sec << " t_tick= " << t_tick << NL;

  return 0;
}

MoveInfo::Ret MoveInfo::calc_step( xfloat a )
{
  if( step_pfun == nullptr ) {
    return Ret::err;
  }
  return step_pfun( *this, a );
}

// not a member - to allow external funcs
MoveInfo::Ret step_line_fun( MoveInfo &mi, xfloat a )
{
  bool need_move = false;
  ranges::fill( mi.cdirs, 0 );

  for( unsigned i=0; i<mi.n_coo; ++i ) { // TODO: common?

    xfloat xf = a * mi.k_x[i];
    int xi = xf * s_movers[i].tick2mm * me_st.axis_scale[i]; // TODO: remove me_st from here?!!
    if( xi == mi.ci[i] ) { // no move in integer approx
        continue;
    }
    need_move = true;

    int dir = ( xi > mi.ci[i] ) ? 1 : -1;
    mi.cdirs[i] = dir;
  }

  return need_move ?  MoveInfo::Ret::move : MoveInfo::Ret::nop;
}

MoveInfo::Ret step_circ_fun( MoveInfo &mi, xfloat a )
{
  bool need_move = false;
  ranges::fill( mi.cdirs, 0 );
  // aliases
  xfloat &r_s   { mi.p[0]   };
  xfloat &alp_s { mi.p[1]   };
  xfloat &z_e   { mi.p[5]   };
  xfloat &e_e   { mi.p[6]   };
  xfloat &x_r   { mi.p[8]   };
  xfloat &y_r   { mi.p[9]   };
  xfloat &k_alp { mi.k_x[0] };
  xfloat &k_r   { mi.k_x[1] };

  xfloat alp =  alp_s + a * k_alp;
  xfloat r   =  r_s   + a * k_r;

  xfloat xx[mi.n_coo];
  xx[0]  = x_r + r * cos( alp );
  xx[1]  = y_r + r * sin( alp );
  xx[2]  = a * z_e;
  xx[3]  = a * e_e;

  for( unsigned i=0; i<mi.n_coo; ++i ) { // TODO: common?

    int xi = xx[i] * s_movers[i].tick2mm * me_st.axis_scale[i]; // TODO: remove me_st from here?!!
    if( xi == mi.ci[i] ) { // no move in integer approx
        continue;
    }
    need_move = true;

    int dir = ( xi > mi.ci[i] ) ? 1 : -1;
    mi.cdirs[i] = dir;
  }

  return need_move ?  MoveInfo::Ret::move : MoveInfo::Ret::nop;
}

// -------------------------- Machine ----------------------------------------------------

// G: val * 1000, to allow G11.123
// M: 1000000 + val * 1000
const Machine::FunGcodePair mg_code_funcs[] = {
  {       0, &Machine::g_move_line     , "move fast X Y Z..." },
  {    1000, &Machine::g_move_line     , "move X Y Z E F_feed S_spin" },
  {    2000, &Machine::g_move_circle   , "circle X Y Z [ I J ] R (CV)" },
  {    3000, &Machine::g_move_circle   , "circle X Y Z [ I J ] R (CCV)" },
  {    4000, &Machine::g_wait          , "wait S_ms P_s" },
  {   17000, &Machine::g_set_plane     , "set plane XY -" },
  {   20000, &Machine::g_set_unit_inch , "set Inch" },
  {   21000, &Machine::g_set_unit_mm   , "set mm" },
  {   28000, &Machine::g_home          , "home X Y Z" },
  {   40000, &Machine::g_off_compens   , "off compens -" },
  {   90000, &Machine::g_set_absmove   , "set ABS move" },
  {   91000, &Machine::g_set_relmove   , "set REL move" },
  {   92000, &Machine::g_set_origin    , "set origin XYZ" },
  { 1000000, &Machine::m_end0           , "end0" },
  { 1001000, &Machine::m_pause          , "pause ?" },
  { 1002000, &Machine::m_end            , "end" },
  { 1003000, &Machine::m_set_spin       , "on spin CV S" },
  { 1004000, &Machine::m_set_spin       , "on spin CCV S" },
  { 1005000, &Machine::m_spin_off       , "off spin" },
  { 1114000, &Machine::m_out_where      , "where" },
  { 1117000, &Machine::m_out_str        , "out string" },
  { 1220000, &Machine::m_set_feed_scale , "set feed scale S %" },
  { 1221000, &Machine::m_set_spin_scale , "set spin scale S U_max %" },
  { 1450000, &Machine::m_out_mode       , "out mode" },
  { 1451000, &Machine::m_set_mode_fff   , "mode_FFF" },
  { 1452000, &Machine::m_set_mode_laser , "mode_laser" },
  { 1453000, &Machine::m_set_mode_cnc   , "mode_cnc" },
};


Machine::Machine( StepMover *a_movers, unsigned a_n_movers )
     : movers( a_movers ), n_movers( a_n_movers ),
       mg_funcs( mg_code_funcs ), mg_funcs_sz( std::size( mg_code_funcs ) )
{
  ranges::fill( axis_scale, 1 );
}

int Machine::step( unsigned i, int dir )
{
  if( i >= n_movers ) {
    return 0;
  }
  if( dir == 0 ) {
    return 0;
  }
  movers[i].step_dir( dir );

  return 1;
}

int Machine::check_endstops( MoveInfo &mi )
{
  // TODO: touch sensor
  for( unsigned i=0; i<mi.n_coo && i<n_movers; ++i ) {
    if( movers[i].endstops == nullptr ) {
      continue;
    }
    uint16_t esv = movers[i].endstops->read();
    const auto dir = mi.cdirs[i];
    // TODO: special case here: request to stop at clear endstop
    if( i == on_endstop && is_endstop_clear(esv ) ) {
      return 0;
    }
    if( is_endstop_clear_for_dir( esv, dir ) ) {
      continue;
    }
    std_out << "# Error: endstop " << esv << " at " << i << " dir: " << dir << NL;
    return 0;
  }
  return 1;
}

int Machine::move_common( MoveInfo &mi, xfloat fe_mmm )
{
  if( mi.type == MoveInfo::Type::stop || mi.step_pfun == nullptr ) {
    return 1;
  }

  fe_mmm *= fe_scale / 100;
  fe_mmm = clamp( fe_mmm, 2.0f, fe_g0 );
  const xfloat k_t { 1.0f / TIM6_count_freq };

  tim_mov_tick = 0; break_flag = 0;

  wait_next_motor_tick();
  const uint32_t tm_s = HAL_GetTick();

  xfloat last_a { -1 };
  int rc {0};
  bool keep_move { true };
  // really must be more then t_tick
  for( unsigned tn=0; tn < 5*mi.t_tick && keep_move && break_flag == 0; ++tn ) {

    leds[2].set();
    xfloat t = tn * k_t;
    xfloat a = t / mi.t_sec; // TODO: accel here
    if( a > 1.0f ) {
      a = 1.0f; keep_move = false;
    }

    auto rc_calc = mi.calc_step( a );
    last_a = a;

    if( check_endstops( mi ) == 0 ) {
      rc = 2;
      break;
    }

    leds[2].reset();

    if( wait_next_motor_tick() != 0 ) { // TODO: loop untill ready + some payload
      rc = 111;
      break;
    }

    if( rc_calc == MoveInfo::Ret::nop ) {
      continue;
    }
    if( rc_calc != MoveInfo::Ret::move ) {
      rc = 3; UVAR('r') = (int)rc_calc;
      break; // TODO: keep reason
    }

    for( unsigned i=0; i<mi.n_coo; ++i ) {
      step( i, mi.cdirs[i] );
      mi.ci[i] += mi.cdirs[i]; // inside post step ???????? OR use StepMover::get_x
    }

  }
  leds[2].reset();
  const uint32_t tm_e = HAL_GetTick();

  OUT << "# debug: move_common: a= " << last_a << " dt= " << ( tm_e - tm_s ) << NL;

  on_endstop = 9999;
  return rc;
}

int Machine::move_line( const xfloat *d_mm, unsigned n_coo, xfloat fe_mmm, unsigned a_on_endstop )
{
  MoveInfo mi( MoveInfo::Type::line, n_coo, step_line_fun );

  auto rc_prep =  mi.prep_move_line( d_mm, fe_mmm );

  if( rc_prep != 0 ) {
    std_out << "# Error: fail to prepare MoveInfo for line, rc=" << rc_prep << NL;
    return 1;
  }

  on_endstop = a_on_endstop;
  return move_common( mi, fe_mmm );
}

// coords: [0]:r_s, [1]: alp_s, [2]: r_e, [3]: alp_e, [4]: cv?, [5]: z_e, [6]: e_e, [7]: nt(L) [8]: x_r, [9]: y_r
int Machine::move_circ( const xfloat *coo, unsigned n_coo, xfloat fe_mmm )
{
  MoveInfo mi( MoveInfo::Type::circle, n_coo, step_circ_fun );
  auto rc_prep =  mi.prep_move_circ( coo, fe_mmm );
  if( rc_prep != 0 ) {
    std_out << "# Error: fail to prepare MoveInfo for circle, rc=" << rc_prep << NL;
    return 1;
  }
  return move_common( mi, fe_mmm );
}

int Machine::g_move_line( const GcodeBlock &gc )
{
  const unsigned n_mo { 4 }; // TODO? who set
  const xfloat meas_scale = inchUnit ? 25.4f : 1.0f;

  xfloat prev_x[n_mo], d_mm[n_mo];
  for( unsigned i=0; i<n_mo; ++i ) {
    prev_x[i] = relmove ? 0 : ( movers[i].get_xf() / meas_scale );
  }

  static constexpr const char *const axis_chars = "XYZEVUW?";
  for( unsigned i=0; i<n_mo; ++i ) {
    d_mm[i] = meas_scale * gc.fpv_or_def( axis_chars[i], prev_x[i] );
  }

  if( !relmove ) {
    for( unsigned i=0; i<n_mo; ++i ) {
      d_mm[i] -= prev_x[i];
    }
  }

  int g = gc.ipv_or_def( 'G', 1 ); // really no check for other: may be used as fallback
  bool g1 { g != 0 };

  xfloat fe_mmm = g1 ? fe_g1 : fe_g0; // TODO: split: 2 different functions

  OUT << "# G" << (g1?'1':'0') << " ( ";
  for( auto xx: d_mm ) {
    OUT << xx << ' ';
  }
  OUT << " ); fe= "<< fe_mmm << NL;

  if( mode == Machine::modeLaser && g1 && spin > 0 ) {
    const xfloat v = getPwm();
    OUT << "# spin= " << spin << " v= " << v << NL;
    pwm_set( 0, v );
  }

  int rc = move_line( d_mm, n_mo, fe_mmm );

  if( mode == Machine::modeLaser ) {
    pwm_off( 0 );
  }
  OUT << "#  G0G1 rc= "<< rc << " break_flag= " << break_flag << NL;

  return rc == 0 ? GcodeBlock::rcOk : GcodeBlock::rcErr;
}

int Machine::g_move_circle( const GcodeBlock &gc )
{
  const unsigned n_mo { 4 }; // TODO? who set
  const xfloat meas_scale = inchUnit ? 25.4f : 1.0f;

  xfloat prev_x[n_mo], coo[8];
  for( unsigned i=0; i<n_mo; ++i ) {
    prev_x[i] = relmove ? 0 : ( movers[i].get_xf() / meas_scale );
  }

  // TODO: data-control
  xfloat x_e = meas_scale * gc.fpv_or_def( 'X', prev_x[0] );
  xfloat y_e = meas_scale * gc.fpv_or_def( 'Y', prev_x[1] );
  xfloat z_e = meas_scale * gc.fpv_or_def( 'Z', prev_x[2] );
  xfloat e_e = meas_scale * gc.fpv_or_def( 'E', prev_x[3] );
  xfloat x_r = meas_scale * gc.fpv_or_def( 'I', NAN );
  xfloat y_r = meas_scale * gc.fpv_or_def( 'J', NAN );
  xfloat r_1 = meas_scale * gc.fpv_or_def( 'R', NAN );
  xfloat nt  = meas_scale * gc.fpv_or_def( 'L', 0 );
  bool cv    = ( gc.ipv_or_def( 'G', 2 ) == 2 );

  if( !relmove ) {
    x_e -= prev_x[0]; y_e -= prev_x[1]; z_e -= prev_x[2]; e_e -= prev_x[3];
  }
  // here only relative value

  bool full_circ = fabsxf( x_e ) < near_l && fabsxf( y_e ) < near_l;
  bool ij_mode = isfinite( x_r ) && isfinite( y_r );
  bool r_mode = !ij_mode && isfinite( r_1 );

  if( ! ij_mode && ! r_mode ) {
    std_out << "# Nor I,J nor R defined" << NL; // TODO: line?
    return g_move_line( gc );
  }

  xfloat r_s, r_e;
  if( ij_mode ) {
    r_s = hypotxf( x_r, y_r );
    r_e = hypotxf( ( x_e - x_r ), ( y_e - y_r ) );
  } else { // R-mode, nor-nor handled before
    if( full_circ ) {
      std_out << "# full circle unavailable in G2/3 R-mode" << NL;
      return g_move_line( gc );
    }
    auto ok = calc_G2_R_mode( cv, x_e, y_e, r_1, x_r, y_r );
    if( !ok ) {
      std_out << "# Error: fail to calc G2/3 R-mode" << NL;
      return g_move_line( gc );
    }
    r_s = r_e = r_1;
  }

  std_out << "# G2/3" << " ( " << x_e << ' ' << y_e << ' ' << z_e
    << " ) c: ( " << x_r << ' ' << y_r << " ) r_1= " << r_1 << ' ' << nt << NL;

  if( r_s < r_min || r_e < r_min || r_s > r_max || r_e > r_max  ) {
    std_out << "# err: bad r, fallback" << NL;
    return g_move_line( gc );
  }


  xfloat alp_s = atan2xf(        -y_r  ,        -x_r   );
  xfloat alp_e = atan2xf( ( y_e - y_r ), ( x_e - x_r ) );


  if( cv ) {
    if( alp_e > alp_s ) {
      alp_e -= M_PIx2;
    }
    if( full_circ ) {
      alp_e = alp_s - M_PIx2;
    }
    alp_e -= nt * M_PIx2;

  } else {
    if( alp_e < alp_s ) {
      alp_e += M_PIx2;
    }
    if( full_circ ) {
      alp_e = alp_s + M_PIx2;
    }
    alp_e += nt * M_PIx2;
  }

  coo[0] = r_s; coo[1] = alp_s; coo[2] = r_e; coo[3] = alp_e;
  coo[4] = cv;  coo[5] = z_e;   coo[6] = e_e; coo[7] = nt;

  xfloat fe_mmm = fe_g1;

  OUT << "# G" << (cv?'2':'3') << " ( ";
  for( auto xx: coo ) {
    OUT << xx << ' ';
  }
  OUT << " ); fe= "<< fe_mmm << NL;

  // TODO: common
  if( mode == Machine::modeLaser && spin > 0 ) {
    const xfloat v = getPwm();
    OUT << "# spin= " << spin << " v= " << v << NL;
    pwm_set( 0, v );
  }

  int rc = move_circ( coo, n_mo, fe_mmm );

  if( mode == Machine::modeLaser ) {
    pwm_off( 0 );
  }
  OUT << "#  G2G3 rc= "<< rc << " break_flag= " << break_flag << NL;
  return rc == 0 ? GcodeBlock::rcOk : GcodeBlock::rcErr;
}

bool calc_G2_R_mode( bool cv, xfloat x_e, xfloat y_e, xfloat &r_1, xfloat &x_r, xfloat &y_r )
{
  int long_arc_k = 1;
  if( r_1 < 0 ) {
    r_1 = -r_1; long_arc_k = -1;
  }
  int ccv_k = cv ? 1 : -1;

  xfloat l_e = hypot( x_e, y_e );
  if( l_e < 1e-3f ) {
    return false;
  }

  xfloat alp_1 = atan2( y_e, x_e );
  xfloat alp_2p = alp_1 + M_PI2;
  xfloat alp_2m = alp_1 - M_PI2;

  xfloat x_c = x_e / 2;
  xfloat y_c = y_e / 2;
  xfloat l_c = hypot( x_c, y_c );

  xfloat h_12 = r_1*r_1 - l_c*l_c;
  if( h_12 < 0 ) {
    std_out << "## Error: h_1 is imaginary!" << NL;
    return false;
  };
  xfloat h_1 = sqrt( h_12 );

  xfloat x_rm = x_c + h_1 * cos( alp_2m );
  xfloat y_rm = y_c + h_1 * sin( alp_2m );
  xfloat x_rp = x_c + h_1 * cos( alp_2p );
  xfloat y_rp = y_c + h_1 * sin( alp_2p );

  // printf( "# rm= ( %g, %g ),   rp= ( %g, %g )\n", x_rm, y_rm, x_rp, y_rp );

  xfloat phi_m_s = atan2( 0.0f - y_rm, 0.0f - x_rm );
  xfloat phi_m_e = atan2( y_e  - y_rm, x_e  - x_rm );
  xfloat phi_p_s = atan2( 0.0f - y_rp, 0.0f - x_rp );
  xfloat phi_p_e = atan2( y_e  - y_rp, x_e  - x_rp );

  // printf( "# phi: %f : %f,   %f : %f\n", r2d(phi_m_s), r2d(phi_m_e), r2d(phi_p_s), r2d(phi_p_e) );

  xfloat d_phi_m = phi_m_e - phi_m_s;
  if( d_phi_m < 0 ) { d_phi_m += M_PIx2; }
  xfloat d_phi_p = phi_p_e - phi_p_s;
  if( d_phi_p < 0 ) { d_phi_p += M_PIx2; }
  xfloat phi_k = ( d_phi_m < d_phi_p ) ? 1 : -1;

  xfloat sel_pm = phi_k * long_arc_k * ccv_k;

  if( sel_pm < 0 ) {
    x_r = x_rm; y_r = y_rm;
  } else {
    x_r = x_rp; y_r = y_rp;
  }

  return true;
}

int Machine::g_wait( const GcodeBlock &gc )
{
  xfloat w = 1000 * gc.fpv_or_def( 'P', 0 );
  w += gc.fpv_or_def( 'S', 1 );
  OUT << "# wait " << w << NL;
  delay_ms_brk( (uint32_t)w );
  return GcodeBlock::rcOk;
}

int Machine::g_set_plane( const GcodeBlock &gc ) // G17
{
  return GcodeBlock::rcOk; // the only plane
}

int Machine::g_set_unit_inch( const GcodeBlock &gc ) // G20
{
  inchUnit = true;
  return GcodeBlock::rcOk;
}

int Machine::g_set_unit_mm( const GcodeBlock &gc ) // G21
{
  inchUnit = false;
  return GcodeBlock::rcOk;
}

int Machine::g_home( const GcodeBlock &gc ) // G28
{
  bool s_x = gc.is_set('X');
  bool s_y = gc.is_set('Y');
  bool s_z = gc.is_set('Z');
  if( !s_x && !s_y && !s_z ) {
    s_x = s_y = s_z = true;
  }
  int rc {0};

  if( s_y ) {
    rc  = go_home( 1 ); // from what?
    if( rc != 1 ) {
      return GcodeBlock::rcErr;
    }
  }

  if( s_x ) {
    rc  = go_home( 0 );
    if( rc != 1 ) {
      return GcodeBlock::rcErr;
    }
  }

  if( s_z ) {
    rc  = go_home( 2 );
    if( rc != 1 ) {
      return GcodeBlock::rcErr;
    }
  }
  was_set = true;

  return rc == 1 ? GcodeBlock::rcOk : GcodeBlock::rcErr;
}

int Machine::g_off_compens( const GcodeBlock &gc ) // G40 - X
{
  OUT << "#  G40 unsupported  " << NL;
  return GcodeBlock::rcOk; // for now
}

int Machine::g_set_absmove( const GcodeBlock &gc ) // G90
{
  relmove = false;
  return GcodeBlock::rcOk;
}

int Machine::g_set_relmove( const GcodeBlock &gc ) // G91
{
  relmove = true;
  return GcodeBlock::rcOk;
}

int Machine::g_set_origin( const GcodeBlock &gc ) // G92
{
  static const char axis_chars[] { 'X', 'Y', 'Z', 'E', 'V' }; // TODO: common + pair? beware: no ""!
  bool a { false }, none_set { true };

  for( char c : axis_chars ) {
    if( gc.is_set( c ) ) {
      none_set = false;
      break;
    }
  }

  unsigned i {0};
  for( char c : axis_chars ) {
    if( i >= n_movers ) {
      break;
    }
    if( gc.is_set( c ) || none_set ) {
      xfloat v = gc.fpv_or_def( c, 0 );
      movers[i].set_xf( v );
      a = true;
    }
    ++i;
  }

  if( a ) {
    was_set = true; // do not change if a false
  }

  return GcodeBlock::rcOk;
}

int Machine::m_end0( const GcodeBlock &gc )          // M0
{
  pwm_set( 0, 0 );
  motors_off();
  return GcodeBlock::rcEnd;
}

int Machine::m_pause( const GcodeBlock &gc )         // M1
{
  // TODO: pause for what?
  OUT << "# pause?" << NL;
  return GcodeBlock::rcOk;
}

int Machine::m_end( const GcodeBlock &gc )           // M2
{
  pwm_set( 0, 0 );
  motors_off();
  return GcodeBlock::rcEnd;
}

int Machine::m_set_spin( const GcodeBlock &gc )      // M3, M4
{
  spin = gc.fpv_or_def( 'S', spin );
  const xfloat v = getPwm();
  OUT << "# M3 " << v << NL;
  spinOn = true;
  if( mode == Machine::modeCNC ) {
    pwm_set( 0, v ); // no direction
    OUT << '+';
  }
  OUT << NL;
  return GcodeBlock::rcOk;
}

int Machine::m_spin_off( const GcodeBlock &gc )      // M5
{
  OUT << "# M5 " << NL;
  pwm_off( 0 );
  spinOn = false;
  return GcodeBlock::rcOk;
}

int Machine::m_out_where( const GcodeBlock &gc )     // M114
{
  // const char axis_chars[] { "XYZEV?" };
  xfloat k_unit = 1.0f / ( inchUnit ? 25.4f : 1.0f );

  for( unsigned i=0; i<n_movers; ++i ) { // TODO: n_active_motor
    OUT << ' ' << movers[i].get_xf() / k_unit;
  }
  OUT << NL "# F= " << fe_g1 << " S= " << spin << " / " << spin100 << NL;

  return GcodeBlock::rcOk;
}

int Machine::m_out_str( const GcodeBlock &gc )       // M117
{
  OUT << "# M117" << NL;
  OUT << gc.get_str0() << NL;
  return GcodeBlock::rcOk;
}

int Machine::m_set_feed_scale( const GcodeBlock &gc )// M220
{
  fe_scale = gc.fpv_or_def( 'S', fe_scale );
  OUT << "# M220 feed scale " << fe_scale << NL;
  return GcodeBlock::rcOk;
}

int Machine::m_set_spin_scale( const GcodeBlock &gc )// M221
{
  spin100  = gc.fpv_or_def( 'S', spin100 );
  spin_max = gc.fpv_or_def( 'U', spin_max );
  OUT << "# M221 spindle scale " << spin100 << ' ' << spin_max << NL;
  return GcodeBlock::rcOk;
}

int Machine::m_out_mode( const GcodeBlock &gc )      // M450
{
  static const char* const mode_names[] = { "FFF", "Laser", "CNC", "??3", "??4", "??5" };
  OUT << "PrinterMode:" << mode_names[mode] << NL;
  return GcodeBlock::rcOk;
}

int Machine::m_set_mode_fff( const GcodeBlock &gc )  // M451
{
  OUT << "# M451 " << NL;
  set_mode( Machine::modeFFF );
  return GcodeBlock::rcOk;
}

int Machine::m_set_mode_laser( const GcodeBlock &gc )// M452
{
  OUT << "# M452 " << NL;
  set_mode( Machine::modeLaser );
  return GcodeBlock::rcOk;
}

int Machine::m_set_mode_cnc( const GcodeBlock &gc )  // M453
{
  OUT << "# M453 " << NL;
  set_mode( Machine::modeCNC );
  return GcodeBlock::rcOk;
}


int Machine::call_mg( const GcodeBlock &cb )
{
  cb.dump(); // debug
  auto is_g = cb.is_set('G');
  auto is_m = cb.is_set('M');
  if( is_g && is_m ) {
    OUT << "#  MS error: M and G" << NL;
    return GcodeBlock::rcErr; // TODO: err_val: both commands
  }

  auto mach_rc = prep_fun( cb );
  if( mach_rc >= GcodeBlock::rcErr ) {
    return mach_rc;
  }

  char chfun  = is_m ? 'M' : 'G';
  int code    = ( is_m ? 1000000 : 0 ) + 1000 * cb.fpv_or_def( chfun, -10000 );

  auto f = std::find_if( mg_funcs, mg_funcs + mg_funcs_sz, [code]( auto el ) { return el.num == code; } );
  if( f == mg_funcs + mg_funcs_sz ) {
    OUT << "# warn: unsupported " << chfun << ' ' << code << NL;
    return GcodeBlock::rcOk;
  }

  auto fun = f->fun;
  auto rc = (this->*fun)( cb );

  if( debug > 0 ) {
    OUT << "# debug: xf:";
    for( unsigned i=0; i<n_movers; ++i ) {
      OUT << ' ' << movers[i].get_xf();
    }
    OUT << NL "# debug: xi:";
    for( unsigned i=0; i<n_movers; ++i ) {
      OUT << ' ' << movers[i].get_x();
    }
    OUT << NL;
  }
  return rc;
}

void Machine::out_mg( bool is_m )
{
  for( unsigned i=0; i<mg_funcs_sz; ++i ) {
    auto [ c, fun, hs ] = mg_funcs[i];
    if( c < 0 ) {
      return;
    }
    if( c < 1000000 ) {
      if( ! is_m ) {
        OUT << 'G' << ( c/1000 ) << ' ' << hs << NL;
      }
      continue;
    }
    if( is_m ) {
      OUT << 'M' << ( ( c-1000000 ) / 1000 ) << ' ' << hs << NL;
    }
  }
}

int Machine::prep_fun(  const GcodeBlock &gc )
{
  if( gc.is_set('M') ) { // special values for M commands
    // OUT << 'M' << NL;
    return GcodeBlock::rcOk;
  }

  bool was_out { false };
  if( gc.is_set('F') ) {
    xfloat v = gc.fpv_or_def( 'F', 100 );
    fe_g1 = v;
    OUT << " F= " << v;
    was_out = true;
  }

  if( gc.is_set('S') ) {
    xfloat v = gc.fpv_or_def( 'S', 1 );
    spin = v;
    OUT << " S= " << v;
    was_out = true;
  }
  if( was_out ) {
    OUT << NL;
  }
  return GcodeBlock::rcOk;
}

// --------------------------- PWM ----------------------------------------


int pwm_set( unsigned idx, xfloat v )
{
  if( idx >= pwm_tims.size() ) {
    return 0;
  }
  auto ti = pwm_tims[idx]->Instance;
  uint32_t arr = ti->ARR;
  uint32_t ccr = (uint32_t)( arr * v / 100.0f );
  ti->CCR1 = ccr;
  ti->CNT  = 0;
  // HAL_TIM_PWM_Start( &htim3, TIM_CHANNEL_1 );
  return 1;
}

int pwm_off( unsigned idx )
{
  if( idx >= pwm_tims.size() ) {
    return 0;
  }
  auto ti = pwm_tims[idx]->Instance;
  ti->CCR1 = 0;
  ti->CNT  = 0;
  //  HAL_TIM_PWM_Stop( &htim3, TIM_CHANNEL_1 );
  return 1;
}

int pwm_off_all()
{
  for( auto t : pwm_tims ) {
    if( t && t->Instance ) {
      t->Instance->CCR1 = 0;
      t->Instance->CNT  = 0;
    }
  }
  return 1;
}


int cmd_pwr( int argc, const char * const * argv )
{
  int    ch  = arg2long_d(  1, argc, argv, 0, 0, pwm_tims.size() ); // including limit for ALL_OFF
  xfloat pwr = arg2xfloat_d( 2, argc, argv, 0, 0.0f, 100.0f );

  if( (unsigned)ch >= pwm_tims.size() ) {
    pwm_off_all();
    std_out << "# PWR: stop all" << NL;
    return 0;
  }

  std_out << "# PWR: ch: " << ch << " power: " << pwr << NL;
  int rc  = pwm_set( ch, pwr );
  // tim_print_cfg( pwm_tims[ch]->Instance );
  return rc == 1 ? 0 : 1;
}

// ------------------------------ EXTI handlers -------------------

void HAL_GPIO_EXTI_Callback( uint16_t pin_bit )
{
  ++UVAR('i'); UVAR('b') = pin_bit;
  leds[1].toggle();
  break_flag = 0x000F0000 + pin_bit;
  pwm_off_all();
  // TODO: PWM and break logic
}

void EXTI0_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( BIT0 );
}

void EXTI1_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( BIT1 );
}

void EXTI2_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( BIT2 );
}

void EXTI3_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( BIT3 );
}

void EXTI4_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( BIT4 );
}

void EXTI9_5_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( BIT5 );
  HAL_GPIO_EXTI_IRQHandler( BIT6 );
}

// ----------------------------- timers -------------------------------------------
// see: ~/proj/stm32/cube/f407_coord/Core/Src/tim.c

int MX_PWM_common_Init( unsigned idx )
{
  if( idx >= pwm_tims.size() ) {
    return 0;
  }
  auto ti = pwm_tims[idx];
  if( ti->Instance == nullptr ) {
    return 0;
  }
  auto t = ti->Instance;
  auto psc   = calc_TIM_psc_for_cnt_freq( t, TIM_PWM_base_freq );
  auto arr   = calc_TIM_arr_for_base_psc( t, psc, TIM_PWM_count_freq );

  ti->Init.Prescaler         = psc;
  ti->Init.CounterMode       = TIM_COUNTERMODE_UP;
  ti->Init.Period            = arr;
  ti->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  ti->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( ti ) != HAL_OK ) {
    UVAR('e') = 31;
    return 0;
  }

  if( HAL_TIM_ConfigClockSource( ti,
        const_cast<TIM_ClockConfigTypeDef*>(&def_pwm_CSC) ) != HAL_OK ) {
    UVAR('e') = 32;
    return 0;
  }

  if( HAL_TIM_PWM_Init( ti ) != HAL_OK ) {
    UVAR('e') = 33;
    return 0;
  }

  if( HAL_TIMEx_MasterConfigSynchronization( ti,
        const_cast<TIM_MasterConfigTypeDef*>(&def_pwm_MasterConfig) ) != HAL_OK ) {
    UVAR('e') = 34;
    return 0;
  }

  if( HAL_TIM_PWM_ConfigChannel( ti,
        const_cast<TIM_OC_InitTypeDef*>(&def_pwm_ConfigOC), TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 35;
    return 0;
  }

  // start here
  HAL_TIM_PWM_Start( ti, TIM_CHANNEL_1 );

  HAL_TIM_MspPostInit( ti );
  return 1;
}

int MX_TIM3_Init()
{
  htim3.Instance = TIM3;
  return MX_PWM_common_Init( 0 );
}

int MX_TIM10_Init()
{
  htim10.Instance = TIM11;
  return MX_PWM_common_Init( 1 );
}

int MX_TIM11_Init()
{
  htim11.Instance = TIM11;
  return MX_PWM_common_Init( 2 );
}

int MX_TIM6_Init()
{
  auto psc   = calc_TIM_psc_for_cnt_freq( TIM6, TIM6_base_freq );       // 83
  auto arr   = calc_TIM_arr_for_base_psc( TIM6, psc, TIM6_count_freq );
  htim6.Instance               = TIM6;
  htim6.Init.Prescaler         = psc;
  htim6.Init.Period            = arr;
  htim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim6 ) != HAL_OK ) {
    UVAR('e') = 61;
    return 0;
  }

  if( HAL_TIMEx_MasterConfigSynchronization( &htim6,
        const_cast<TIM_MasterConfigTypeDef*>(&def_pwm_MasterConfig) ) != HAL_OK ) {
    UVAR('e') = 62;
    return 0;
  }
  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if(      tim_baseHandle->Instance == TIM2 ) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  }
  else if( tim_baseHandle->Instance == TIM3 ) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  }
  else if( tim_baseHandle->Instance == TIM4 ) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    GpioD.cfgAF( 13, GPIO_AF2_TIM4 );  // TIM4.2 D13 se3
    GpioD.cfgAF( 14, GPIO_AF2_TIM4 );  // TIM4.3 D13 se2
    GpioD.cfgAF( 15, GPIO_AF2_TIM4 );  // TIM4.4 D13 se1
    // HAL_NVIC_SetPriority( TIM4_IRQn, 5, 0 );
    // HAL_NVIC_EnableIRQ( TIM4_IRQn );
  }
  else if( tim_baseHandle->Instance == TIM6 ) {
    __HAL_RCC_TIM6_CLK_ENABLE();
    HAL_NVIC_SetPriority( TIM6_DAC_IRQn, 5, 0 );
    HAL_NVIC_EnableIRQ(   TIM6_DAC_IRQn );
  }
  else if( tim_baseHandle->Instance == TIM10 ) {
    __HAL_RCC_TIM10_CLK_ENABLE();
  }
  else if( tim_baseHandle->Instance == TIM11 ) {
    __HAL_RCC_TIM11_CLK_ENABLE();
  }
}

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle )
{
  if(      timHandle->Instance == TIM2 ) {
    GpioB.cfgAF( 10, GPIO_AF1_TIM2 );  // TIM2.3: B10 PWM3? aux1.6
    GpioB.cfgAF( 11, GPIO_AF1_TIM2 );  // TIM2.4: B11 PWM4? aux1.8
  }
  else if( timHandle->Instance == TIM3 ) {
    GpioC.cfgAF( 6, GPIO_AF2_TIM3 );  // TIM3.1:  C6 PWM0
  }
  else if( timHandle->Instance == TIM10 ) {
    GpioB.cfgAF( 8, GPIO_AF3_TIM10 ); // TIM10.1: B8 PWM1
  }
  else if( timHandle->Instance == TIM11 ) {
    GpioB.cfgAF( 9, GPIO_AF3_TIM11 ); // TIM11.1: B9 PWM2
  }
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if(      tim_baseHandle->Instance == TIM2  ) {
    __HAL_RCC_TIM2_CLK_DISABLE();
  }
  else if( tim_baseHandle->Instance == TIM3  ) {
    __HAL_RCC_TIM3_CLK_DISABLE();
  }
  else if( tim_baseHandle->Instance == TIM4  ) {
    __HAL_RCC_TIM4_CLK_DISABLE();
    HAL_NVIC_DisableIRQ( TIM4_IRQn );
  }
  else if( tim_baseHandle->Instance == TIM6  ) {
    __HAL_RCC_TIM6_CLK_DISABLE();
    HAL_NVIC_DisableIRQ( TIM6_DAC_IRQn );
  }
  else if( tim_baseHandle->Instance == TIM10 ) {
    __HAL_RCC_TIM10_CLK_DISABLE();
  }
  else if( tim_baseHandle->Instance == TIM11 ) {
    __HAL_RCC_TIM11_CLK_DISABLE();
  }
}

void TIM4_IRQHandler()
{
  HAL_TIM_IRQHandler( &htim4 );
}

void TIM6_DAC_IRQHandler()
{
  HAL_TIM_IRQHandler( &htim6 );
}

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
  if( htim->Instance == TIM6 ) {
    TIM6_callback();
    return;
  }
}

void TIM6_callback()
{
  tim_mov_tick = 1;
  return;
}

// ------------------------------------------------------------
int wait_next_motor_tick()
{
  while( tim_mov_tick == 0 && break_flag == 0 ) {
    /* NOP */ // TODO: idle action
  }
  tim_mov_tick = 0;
  return break_flag;
}

void StepMover::set_dir( int a_dir )
{
  if( a_dir == dir ) {
    return;
  }
  dir = a_dir;

  if( motor ) {
    if( dir >= 0 ) {
      motor->reset( 0x02 );
    } else {
      motor->set(   0x02 );
    }
    delay_mcs( 1 );
  }
}

void StepMover::step()
{
  if( motor ) {
    motor->set( 1 );
    delay_mcs( 1 );
    motor->reset( 1 );
  }
  x += dir;
}

// ----------------------------------------  ------------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

