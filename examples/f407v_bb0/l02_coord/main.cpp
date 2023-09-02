#include <cerrno>
#include <climits>
#include <cmath>
#include <cstring>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_atleave.h>
#include <oxc_fs_cmd0.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
//#include <oxc_outstr.h>
//#include <oxc_hd44780_i2c.h>
//#include <oxc_menu4b.h>
//#include <oxc_statdata.h>
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


MachParam machs[n_motors] = {
  {  1600, 500,     150,     &x_e, &stepdir_x  }, // TODO: 500 increase after working acceleration
  {  1600, 500,     300,     &y_e, &stepdir_y  },
  {  1600, 300,     150,     &z_e, &stepdir_z  },
  {   100, 100,  999999,  nullptr, &stepdir_e0 },
  {   100, 100,  999999,  nullptr, &stepdir_e1 }
};


MachState::MachState( fun_gcode_mg prep, const FunGcodePair *g_f, const FunGcodePair *m_f )
     : MachStateBase( prep, g_f, m_f )
{
  for( auto &xx : x ) { xx = 0; }
  for( auto &s : axis_scale ) { s = 1; }
}

MachState me_st( mach_prep_fun, mach_g_funcs, mach_m_funcs );

MoveTask1 move_task[n_motors+1]; // last idx = time



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
CmdInfo CMDINFO_RELMOVE { "rel", 'R', cmd_relmove, "dx dy dz [feed] - rel move test"  };
int cmd_absmove( int argc, const char * const * argv );
CmdInfo CMDINFO_ABSMOVE { "abs", 'A', cmd_absmove, "x y z [feed] - abs move test"  };
int cmd_home( int argc, const char * const * argv );
CmdInfo CMDINFO_HOME { "home", 'H', cmd_home, "axis - go home at give axis"  };
int cmd_pwr( int argc, const char * const * argv );
CmdInfo CMDINFO_PWR { "pwr", 'P', cmd_pwr, "ch pow_f  - test PWM power control"  };
int cmd_zero( int argc, const char * const * argv );
CmdInfo CMDINFO_ZERO { "zero", 'Z', cmd_zero, "[x y z]  - set [zero] point"  };
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
  &CMDINFO_ZERO,
  &CMDINFO_GEXEC,
  &CMDINFO_FIRE,
  nullptr
};

const constinit NamedInt   ob_break_flag    {    "break_flag",    const_cast<int*>(&break_flag)  }; // may be UB?
//const NamedFloat ob_f    {    "pwm_min",      get_pwm_min,  set_pwm_min  };

const constinit NamedObj *const objs_info[] = {
  & ob_break_flag,
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

// handle Gnnn...., Mnnn..., !SOME_OTHER_COMMAND
int gcode_cmdline_handler( char *s )
{
  if( !s ) {
    return -1;
  };
  if( s[0] != '!' && s[0] != 'G' && s[0] != 'M' ) { // not my
    return -1;
  };

  const char *cmd = s;
  if( cmd[0] == '!' ) { // skip '!'
    ++cmd;
  }
  std_out << NL "# gcode: cmd= \"" << cmd << '"' << NL;
  delay_ms( 10 );

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();

  GcodeBlock cb ( &me_st );
  int rc = cb.process( cmd );
  // if( rc >= GcodeBlock::rcEnd ) {
    std_out << "# rc " << rc << " line \"" << cmd << "\" pos " << cb.get_err_pos() << NL;
  //}

  return 0;
}

void idle_main_task()
{
  const uint32_t ep_mask = 0b01111011;
  auto epv = GpioD.IDR & ep_mask;
  leds[1] = ( epv == ep_mask ) ? 0 : 1;
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

  for( auto &m : machs ) {
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

  cmdline_handlers[0] = gcode_cmdline_handler;
  cmdline_handlers[1] = nullptr;

  // just for now
  me_st.mode = MachState::modeLaser;

  BOARD_POST_INIT_BLINK;

  leds.reset( 0xFF );


  pr( NL "##################### " PROJ_NAME NL );

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

  auto motor = machs[a].motor;
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

int move_rel( const float *d_mm_i, unsigned n_mo,  float fe_mmm )
{
  for( auto &m : move_task )      { m.init(); }
  fe_mmm *= me_st.fe_scale / 100;
  fe_mmm = clamp( fe_mmm, 2.0f, me_st.fe_g0 );
  UVAR('k') = UVAR('l') = UVAR('x') = UVAR('y') = UVAR('z') = 0;

  me_st.n_mo = n_mo;
  float d_mm[n_mo];
  for( unsigned i=0; i<n_mo; ++i ) { d_mm[i] = d_mm_i[i]; } // local copy, as we change it: round(step)-sign

  float feed3_max { 0 }; // mm/min
  float d_l { 0 };
  int max_steps { 0 }, max_steps_idx { 0 };

  delay_ms( UVAR('v') ); // for filming

  // calculate number of steps on each axis
  for( unsigned i=0; i<n_mo; ++i ) {
    const float dc = d_mm[i];
    const float step_sz = 1.0f / ( machs[i].tick2mm * me_st.axis_scale[i] );
    if( dc > 0.5f * step_sz ) {
      move_task[i].dir =  1;
      move_task[i].step_task = move_task[i].step_rest =  roundf( dc / step_sz );
      machs[i].motor->sr( 0x02, 0 );
    } else if( dc < -0.5f * step_sz ) {
      move_task[i].dir = -1;
      move_task[i].step_task = move_task[i].step_rest = -roundf( dc / step_sz );
      machs[i].motor->sr( 0x02, 1 );
    } else {
      // move_task[i].dir =  0; // zeroed before
      machs[i].motor->sr( 0x02, 0 ); // just to be determined
    }
    if( max_steps < move_task[i].step_rest ) {
      max_steps   = move_task[i].step_rest;
      max_steps_idx = i;
    }

    const float d_c = move_task[i].step_rest * step_sz;
    d_mm[i] = d_c;
    std_out << "# " << i << ' ' << dc << ' ' << d_c
            << move_task[i].dir << ' ' << move_task[i].step_rest << NL;
    d_l += d_c * d_c;
    feed3_max += machs[i].max_speed * machs[i].max_speed;
  }

  d_l = sqrtf( d_l );
  feed3_max = sqrtf( feed3_max );
  if( fe_mmm > feed3_max ) {
    fe_mmm = feed3_max;
  }

  std_out << "# feed= " << fe_mmm << " mm/min; " << " d_l= " << d_l
          << " mm;  feed3_max= " << feed3_max
          << "  max_steps= " << max_steps  << ' ' << max_steps_idx << NL;

  float feed[n_mo];
  float feed_lim { 0 };
  for( unsigned i=0; i<n_mo; ++i ) {
    feed[i] = fe_mmm * d_mm[i] / d_l;
    if( feed[i] > machs[i].max_speed ) {
      feed[i] =   machs[i].max_speed;
    }
    feed_lim += feed[i] * feed[i];
    // std_out << "# feed[" << i << "]= " << feed[i] << NL;
  }
  feed_lim = sqrtf( feed_lim );

  std_out << "# feed_lim= " << feed_lim << NL;
  if( feed[max_steps_idx] < 1e-3f || feed_lim < 1e-3f ) {
    std_out << "# Error: too low speed, exiting" << NL;
    return 5;
  }

  float t_all = 60 * d_mm[max_steps_idx] / feed[max_steps_idx];
  uint32_t t_all_tick = 2 + ( uint32_t( t_all * TIM6_count_freq ) & ~1u );
  std_out << "# t_all= " << t_all << " s = " <<  t_all_tick << NL;

  if( t_all_tick < 3 ) {
    std_out << "# jump on place" << NL;
    delay_ms( 1 );
    return 0;
  }

  move_task[n_motors].step_rest = move_task[n_motors].step_task = t_all_tick;

  // check endstops
  for( unsigned i=0; i<n_mo; ++i ) {
    if( machs[i].endstops == nullptr ) {
      continue;
    }
    uint16_t epv = machs[i].endstops->read();
    const auto dir = move_task[i].dir;
    // std_out << "# debug: endstop " << epv << " at " << i << " dir: " << dir << NL;
    if( dir == 0 || ( epv & 0x03 ) == 0x03 ) { // no move or all clear
      continue;
    }
    if( dir >  0 && ( epv & 0x02 ) ) { // forward and ep+ clear
      continue;
    }
    if( dir <  0 && ( epv & 0x01 ) ) { // backward and ep- clear
      continue;
    }
    std_out << "# Error: endstop " << epv << " at " << i << " dir: " << dir << NL;
    return 7;
  }

  HAL_TIM_Base_Start_IT( &htim6 );
  uint32_t tm0 = HAL_GetTick(), tc0 = tm0;

  break_flag = 0;
  int t_est = (int)( t_all * 11 + 5 ); // + 10%, + 0.5s
  for( int i=0; i<t_est && !break_flag; ++i ) {

    leds[3].toggle();

    delay_ms_until_brk( &tc0, 100 );

  }
  HAL_TIM_Base_Stop_IT( &htim6 ); // may be other breaks, so dup here

  me_st.last_rc = break_flag;

  float real_d[n_mo], l_err { 0.0f };
  for( unsigned i=0; i<n_mo; ++i ) {
    real_d[i] = float( move_task[i].step_task - move_task[i].step_rest ) * move_task[i].dir
      / ( machs[i].tick2mm * me_st.axis_scale[i] );
    me_st.x[i] += real_d[i];
    l_err += pow2f( d_mm_i[i] - real_d[i] );
    std_out << " d_" << i << " = " << real_d[i] << " x= " << me_st.x[i] << NL;
  }
  l_err = sqrtf( l_err );
  std_out << "# l_err= " << l_err;

  if( l_err > 0.1f ) {
    return 9;
  }

  return ( me_st.last_rc == 2 ) ? 0 : 8;
}

int go_home( unsigned axis )
{
  if( axis >= n_motors ) {
    std_out << "# Error: bad axis index " << axis << NL;
    return 0;
  }
  auto estp = machs[axis].endstops;
  if( machs[axis].max_l > 20000 || estp == nullptr ) {
    std_out << "# Error: unsupported axis  " << axis << NL;
    return 0;
  }

  pwm_off_all();
  me_st.was_set = false;

  const unsigned n_mo { 3 }; // 3 = only XYZ motors
  float d_mm[n_mo];
  for( auto &x : d_mm ) { x = 0; }

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();

  // TODO: go from endstops, params

  float fe_quant = (float)machs[axis].max_speed / 20;
  d_mm[axis] = -2.0f * (float)machs[axis].max_l;

  int rc = move_rel( d_mm, n_mo, 16 * fe_quant );
  auto esv = estp->read();
  if( rc != 9 || esv != 2 ) { // must be bad d_l
    std_out << "# Error: fail to find endstop axis " << axis << " ph 1 esv " << esv << NL;
    return 0;
  }

  d_mm[axis] = 2.0f;
  rc = move_rel( d_mm, n_mo, 2 * fe_quant );
  esv = estp->read();
  if( rc != 0 || esv != 3 ) { // must be ok
    std_out << "# Error: fail to step from endstop axis " << axis << " ph 2 esv " << esv << NL;
    return 0;
  }

  d_mm[axis] = -3.0f;
  rc = move_rel( d_mm, n_mo, 2 * fe_quant );
  esv = estp->read();
  if( esv != 2 ) { // must be sens
    std_out << "# Error: fail to find endstop axis " << axis << " ph 3 esv " << esv << NL;
    return 0;
  }

  d_mm[axis] = 0.02f;
  const unsigned n_try = 100;
  bool found = false;
  for( unsigned i=0; i<n_try; ++i ) {
    rc = move_rel( d_mm, n_mo, 2 * fe_quant );
    esv = estp->read();
    if( rc == 0 && esv == 3 ) {
      std_out << "# Ok: found endstop end at try  " << i << NL;
      found = true;
      break;
    }
  }

  if( found ) {
    me_st.was_set = true;
    me_st.x[axis] = 0;    // TODO: may be no auto?
    return 1;
  }

  std_out << "# Error: fail to find endstop axis " << axis << " ph 4 esv " << esv << NL;
  return 0;
}

int cmd_relmove( int argc, const char * const * argv )
{
  std_out << "# relmove: " << NL;
  const unsigned n_mo { 3 }; // 3 = only XYZ motors

  float d_mm[n_mo];

  for( unsigned i=0; i<n_mo; ++i ) {
    d_mm[i] = arg2float_d( i+1, argc, argv, 0, -(float)machs[i].max_l, (float)machs[i].max_l );
  }
  float fe_mmm = arg2float_d( 4, argc, argv, UVAR('f'), 0.0f, 900.0f );

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();
  int rc = move_rel( d_mm, n_mo, fe_mmm );

  return rc;
}

int cmd_absmove( int argc, const char * const * argv )
{
  std_out << "# absmove: " << NL;

  if( ! me_st.was_set ) {
    std_out << "# Error: zero point not set" << NL;
    return 2;
  }

  const unsigned n_mo { 3 }; // 3 = only XYZ motors

  float d_mm[n_mo];

  for( unsigned i=0; i<n_mo; ++i ) {
    d_mm[i] = arg2float_d( i+1, argc, argv, 0, -(float)machs[i].max_l, (float)machs[i].max_l )
            - me_st.x[i];
  }
  float fe_mmm = arg2float_d( 4, argc, argv, UVAR('f'), 0.0f, 900.0f );

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();
  int rc = move_rel( d_mm, n_mo, fe_mmm );

  return rc;
}

int cmd_home( int argc, const char * const * argv )
{
  unsigned axis = arg2long_d( 1, argc, argv, 0, 0, n_motors );

  std_out << "# home: " << axis << NL;

  int rc = go_home( axis );

  return rc;
}

int cmd_zero( int argc, const char * const * argv )
{
  for( unsigned i=0; i<me_st.n_mo; ++i ) {
    me_st.x[i] =  arg2float_d( i+1, argc, argv, 0, -machs[i].max_l, machs[i].max_l );
  }
  me_st.was_set = true;
  return 0;
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
    std_out << "# gexec error: fail ro open file \"" << fn << "\" r= " << r << NL;
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


    GcodeBlock cb ( &me_st );
    int rc = cb.process( s );
    ++nle;
    std_out << "# rc= " << rc << " br= " << break_flag << " nle= " << nle << NL;
    if( break_flag == 2 ) {
      break_flag = 0;
    }

    // TODO: params
    motors_off();
    delay_ms( 50 );
    motors_on();

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
  float pwr = arg2float_d( 1, argc, argv, 0.05, 0.0f, 100.0f );
  unsigned dt = arg2long_d( 2, argc, argv, 0, 0, 10000 );
  std_out << "# fire: pwr= " << pwr << " dt=  " << dt << NL;

  pwm_set( 0, pwr );
  delay_ms_brk( dt );
  pwm_set( 0, 0 );

  return 0;
}

int pwm_set( unsigned idx, float v )
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
  int   ch  = arg2long_d(  1, argc, argv, 0, 0, pwm_tims.size() ); // including limit for ALL_OFF
  float pwr = arg2float_d( 2, argc, argv, 0, 0.0f, 100.0f );

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

void EXTI0_IRQHandler(void)
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
  auto arr   = calc_TIM_arr_for_base_psc( TIM6, psc, TIM6_count_freq ); // 49
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
  if(     tim_baseHandle->Instance == TIM2 ) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  }
  else if(tim_baseHandle->Instance == TIM3 ) {
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
  ++UVAR('k');

  leds[2].set();
  bool do_stop = true;
  for( unsigned i=0; i<me_st.n_mo && break_flag == 0; ++i ) {
    if( move_task[i].dir == 0  ||  move_task[i].step_rest < 1 ) {
      continue;
    }
    move_task[i].d += 2 * move_task[i].step_task ;
    do_stop = false;
    if( move_task[i].d > move_task[n_motors].step_task ) {
      // ++UVAR('x'+i);
      (*machs[i].motor)[0].toggle();
      --move_task[i].step_rest;
      move_task[i].d -= 2 * move_task[n_motors].step_task;
    }
  }

  if( do_stop || break_flag != 0 ) {
    HAL_TIM_Base_Stop_IT( &htim6 );
    break_flag = break_flag ? break_flag : 2;
  }
  leds[2].reset();

  return;
}


// ----------------------------------------  ------------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

