#include <cerrno>
#include <climits>
#include <cmath>
#include <cstring>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_atleave.h>
#include <oxc_fs_cmd0.h>

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

StepMotorGpio2 stepdir_v( STEPDIR_V_GPIO, STEPDIR_V_STARTPIN );
StepMotorGpio2 stepdir_e( STEPDIR_E_GPIO, STEPDIR_E_STARTPIN );
StepMotorGpio2 stepdir_x( STEPDIR_X_GPIO, STEPDIR_X_STARTPIN );
StepMotorGpio2 stepdir_y( STEPDIR_Y_GPIO, STEPDIR_Y_STARTPIN );
StepMotorGpio2 stepdir_z( STEPDIR_Z_GPIO, STEPDIR_Z_STARTPIN );


PinOut en_motors( GpioC, 11 );

void motors_off() {
  en_motors = 1;
}

void motors_on()  {
  en_motors = 0;
}

PinsOut aux3(  GpioD, 7, 4 );

EndStopGpioPos estp_x(  GpioD, 0, 2 );
EndStopGpioPos estp_y(  GpioD, 3, 2 );
EndStopGpioPos estp_z(  GpioD, 5, 2 );



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


StepMover mover_x( stepdir_x, &estp_x, 800, 500, 150 );
StepMover mover_y( stepdir_y, &estp_y, 800, 500, 300 );
StepMover mover_z( stepdir_z, &estp_z, 800, 300, 150 );
StepMover mover_e( stepdir_e, nullptr, 100, 100, 999999 );
StepMover mover_v( stepdir_v, nullptr, 100, 100, 999999 );

StepMover* s_movers[n_motors] { &mover_x, &mover_y, &mover_z, &mover_e, &mover_v };


Machine me_st( s_movers );


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
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", 'P', cmd_pwm, "ch pow_f  - test PWM power control"  };
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
  &CMDINFO_PWM,
  &CMDINFO_GEXEC,
  &CMDINFO_FIRE,
  nullptr
};




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

  DoAtLeave do_off_motors( []() { motors_off(); } ); // ??? param?
  motors_on();

  GcodeBlock cb ( gcode_act_fun_me_st );
  int rc = cb.process( cmd );
  std_out << "# rc " << rc << " line \"" << cmd << "\" pos " << cb.get_err_pos() << NL;

  return 0;
}

ReturnCode gcode_act_fun_me_st( const GcodeBlock &gc )
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

  aux3.initHW(); aux3 = 0;
  en_motors.initHW(); // TODO: to machine
  motors_off();

  EXTI_inits( extis, true );

  MX_TIM3_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_TIM6_Init();

  me_st.initHW();

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

  // print_var_hook = print_var_ex;
  // set_var_hook   = set_var_ex;

  cmdline_handlers[0] = gcode_cmdline_handler;
  cmdline_handlers[1] = nullptr;

  // just for now
  me_st.set_n_mo( 3 );
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
  int mo_idx = arg2long_d( 1, argc, argv, 1, 0, 100000000 ); // motor index
  // int n = arg2long_d( 2, argc, argv, UVAR('n'), -10000000, 100000000 ); // number of pulses with sign
  // uint32_t dt = arg2long_d( 3, argc, argv, UVAR('t'), 0, 1000 ); // ticks in ms

  auto rc = me_st.go_from_es( mo_idx );
  OUT << "# debug: mo_idx= " << mo_idx << " rc= " << rc << NL;

  return rc;
}



int cmd_relmove( int argc, const char * const * argv )
{
  std_out << "# relmove: " << NL;

  xfloat d_mm[n_motors];

  for( unsigned i=0; i<n_motors; ++i ) {
    d_mm[i] = arg2xfloat_d( i+1, argc, argv, 0 );
  }
  xfloat fe_mmm = arg2xfloat_d( 4, argc, argv, UVAR('f'), 0.0f, 900.0f );

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();
  int rc = me_st.move_line( d_mm, fe_mmm );

  return rc;
}

int cmd_absmove( int argc, const char * const * argv )
{
  std_out << "# absmove: " << NL;

  xfloat d_mm[n_motors];

  for( unsigned i=0; i<n_motors; ++i ) {
    d_mm[i] = arg2xfloat_d( i+1, argc, argv, 0 ) - me_st.get_xn( i );
  }
  xfloat fe_mmm = arg2xfloat_d( 4, argc, argv, UVAR('f'), 0.0f, 9000.0f ); // large to test limiting

  DoAtLeave do_off_motors( []() { motors_off(); } );
  motors_on();
  int rc = me_st.move_line( d_mm, fe_mmm );

  return rc;
}



int cmd_home( int argc, const char * const * argv )
{
  unsigned axis = arg2long_d( 1, argc, argv, 0, 0, n_motors );

  std_out << "# home: " << axis << NL;

  ReturnCode rc = me_st.go_home( axis );

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

  errno = 0;
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
    auto rc = cb.process( s );
    ++nle;
    std_out << "# rc= " << rc << " br= " << break_flag << " nle= " << nle << NL;
    if( break_flag == 2 ) {
      break_flag = 0;
    }

    if( rc >= ReturnCode::rcEnd ) {
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


// -------------------------- Machine ----------------------------------------------------

const char Machine::axis_chars[] { "XYZEUVWABC" }; // beware of '\0'

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
  { 1995000, &Machine::m_list_vars      , "list_vars" },
  { 1996000, &Machine::m_set_var        , "set_var S \"\" V v" },
  { 1997000, &Machine::m_get_var        , "get_var S \"\"" },
};

const Machine::VarInfo Machine::var_info[] = {
  { "fe_g0",          &Machine::fe_g0,       nullptr },
  { "fe_g1",          &Machine::fe_g1,       nullptr },
  { "fe_min",         &Machine::fe_min,      nullptr },
  { "fe_scale",       &Machine::fe_scale,    nullptr },
  { "near_l",         &Machine::near_l  ,    nullptr },
  { "dly_xsteps",     nullptr,               &Machine::dly_xsteps },
  { "reen_motors",    nullptr,               &Machine::reen_motors },
};

Machine::Machine( std::span<StepMover*> a_movers )
     : movers( a_movers ),
       mg_funcs( mg_code_funcs ), mg_funcs_sz( std::size( mg_code_funcs ) )
{
  ranges::fill( axis_scale, 1 );
}

void Machine::initHW()
{
  for( auto pm : movers ) {
    if( pm ) {
      pm->initHW();
    }
  }
}


const char* Machine::endstops2str( char *buf ) const
{
  static char sbuf[n_motors+2];
  if( !buf ) {
    buf = sbuf;
  }
  std::fill( buf, buf+sizeof(sbuf), ' ' );
  buf[ sizeof(sbuf) - 1 ] = '\0';

  for( unsigned i=0; i<n_mo; ++i ) {
    if( movers[i]->get_endstops() ) {
      buf[i] =  movers[i]->get_endstops()->toChar();
    }
    buf[i+1] = '\0';
  }
  return buf;
}

const char* Machine::endstops2str_read( char *buf )
{
  for( unsigned i=0; i<n_mo; ++i ) {
    if( movers[i] && movers[i]->get_endstops() ) {
      movers[i]->get_endstops()->read();
    }
  }
  return endstops2str( buf );
}

ReturnCode Machine::move_common( MoveInfo &mi, xfloat fe_mmm )
{
  if( !mi.isGood() ) {
    return rcErr;
  }

  errno = 0;
  const xfloat  fe_max = ( move_mode & moveFast ) ? fe_g0 : fe_g1;
  if( fe_mmm < fe_min ) {
    fe_mmm = fe_max;
  }
  fe_mmm *= fe_scale / 100;
  fe_mmm = clamp( fe_mmm, fe_min, fe_max );
  const xfloat k_t { 1.0f / TIM6_count_freq };

  xfloat t_sec = 60 * mi.len / fe_mmm; // only approx, as we can accel/deccel
  uint32_t t_tick = (uint32_t)( TIM6_count_freq * t_sec );

  tim_mov_tick = 0; break_flag = 0;

  wait_next_motor_tick();
  const uint32_t tm_s = HAL_GetTick();

  xfloat last_a { -1 };
  xfloat coo[movers.size()];
  xfloat o_coo[movers.size()];
  for( unsigned i=0; i<mi.n_coo; ++i ) {
    o_coo[i] = movers[i]->get_xf();
  }
  ReturnCode rc { rcOk };
  bool keep_move { true };
  bool onoff_laser = ( move_mode & moveActive ) && ( mode == modeLaser ) && ( spin > 0 );

  if( onoff_laser ) {
    const xfloat v = getPwm();
    OUT << "# power= " << spin << " v= " << v << NL;
    pwm_set( 0, v );
  }

  // really must be more then t_tick
  for( unsigned tn=0; tn < 5*t_tick && keep_move && break_flag == 0; ++tn ) {

    leds[2].set();
    xfloat t = tn * k_t;
    xfloat a = t / t_sec; // TODO: accel here
    if( a > 1.0f ) {
      a = 1.0f; keep_move = false;
    }

    ReturnCode rc_c = mi.calc_step( a, coo );
    last_a = a;

    leds[2].reset();
    if( rc >= ReturnCode::rcErr ) {
      errno = 2001;
      rc = rc_c;
      break;
    }


    if( wait_next_motor_tick() != 0 ) { // TODO: loop untill ready + some payload
      errno = 2002;
      rc = rcErr;
      break;
    }

    unsigned n_ok { 0 };
    for( unsigned i=0; i<mi.n_coo; ++i ) {
      if( ! ( active_movers_bits & ( 1u << i ) ) ) {
        continue;
      }
      // TODO: bitfield of active motors
      auto rc_s = movers[i]->step_to( coo[i] + o_coo[i] );
      if( rc_s == rcOk ) {
        ++n_ok;
        continue;
      }
      rc = rc_s;
      if( rc_s >= rcErr ) {
        keep_move = false;
        errno = 2003;
        break;
      }
      if( rc_s >= rcEnd && bounded_move ) {
        keep_move = false;
        errno = 2004;
        break;
      }
    }

    if( n_ok < 1 ) {
      if( rc < rcEnd ) {
        rc = rcEnd; // TODO: or worse
      }
      errno = 2005;
      break;
    }

  } // t-loop
  leds[2].reset();

  if( onoff_laser ) {
    pwm_off( 0 );
  }

  const uint32_t tm_e = HAL_GetTick();

  OUT << "# debug: move_common: a= " << last_a << " rc= " << rc << " dt= " << ( tm_e - tm_s )
    << " fe= " << fe_mmm << ' ' << endstops2str_read() << NL;

  if( reen_motors ) {
    motors_off(); // TODO: investigate! + sensors
  }
  if( dly_xsteps > 0 ) {
    delay_ms( dly_xsteps );
  }
  if( reen_motors ) {
    motors_on();
  }

  return rc;
}

ReturnCode Machine::move_line( const xfloat *prm, xfloat fe_mmm )
{
  MoveInfo mi( n_mo, step_line_fun );

  auto rc_prep =  mi.prep_move_line( prm );

  if( rc_prep != 0 ) {
    std_out << "# Error: fail to prepare MoveInfo for line, rc=" << rc_prep << NL;
    return rcErr;
  }

  return move_common( mi, fe_mmm );
}

ReturnCode Machine::move_circ( const xfloat *prm, xfloat fe_mmm )
{
  MoveInfo mi( n_mo, step_circ_fun );
  auto rc_prep =  mi.prep_move_circ( prm );
  if( rc_prep != 0 ) {
    std_out << "# Error: fail to prepare MoveInfo for circle, rc=" << rc_prep << NL;
    return rcErr;
  }
  return move_common( mi, fe_mmm );
}

ReturnCode Machine::go_home( unsigned motor_bits )
{
  if( motor_bits == 0 ) {
    motor_bits = active_movers_bits;
  }

  // calc - coords for given movers
  xfloat coo[n_motors];
  std::ranges::fill( coo, 0 );

  for( unsigned i=0, b=0x01; i<n_mo; ++i, b<<=1 ) {
    if( ( motor_bits & b ) == 0 ) {
      continue;
    }
    auto mover = movers[i];
    if( ! mover ) {
      motor_bits &= ~b; // drop bit from motor_bits for nonexitent
      continue;
    }
    auto estp = mover->get_endstops();
    if( !estp ) {
      motor_bits &= ~b;
      continue;
    }
    mover->set_es_mode( StepMover::EndstopMode::Dir );
    coo[i] = -5000; // 5 m, for equal speed on all axis
  }

  RestoreAtLeave rst_mo_bits( active_movers_bits, motor_bits );

  // fast to all endstops
  bounded_move = false;
  auto rc = move_line( coo, fe_g1 );
  if( rc != rcEnd ) {
    OUT << "# Error: fail to find all endstops: " << endstops2str_read() << ' ' << rc << NL;
    return rcErr;
  }

  for( unsigned i=0, b=0x01; i<n_mo; ++i, b<<=1 ) {
    if( ( motor_bits & b ) == 0 ) {
      continue;
    }
    auto mover = movers[i]; // checked before
    auto estp = mover->get_endstops();
    rc = go_from_es_nc( i, mover, estp );
    OUT << "# debug: from 1/ " << i << ' ' << rc << NL;
    if( rc >= rcErr ) {
      break;
    }
    rc = go_to_es_nc( i, mover, estp );
    OUT << "# debug: from 2/ " << i << ' ' << rc << NL;
    if( rc >= rcErr ) {
      break;
    }
    rc = go_from_es_nc( i, mover, estp );
    OUT << "# debug: from 2/ " << i << ' ' << rc << NL;
    if( rc >= rcErr ) {
      break;
    }
    mover->set_x( 0 );
  }

  // TODO: substep out of?

  bounded_move = true;
  return rc;
}

ReturnCode Machine::go_from_es( unsigned mover_idx )
{
  if( mover_idx >= n_mo ) {
    OUT << "# Err: bad mover index " << mover_idx << ' ' << n_mo << NL;
    return rcErr;
  }
  auto mover = movers[mover_idx];
  if( !mover ) {
    OUT << "# Err: bad mover ptr " << mover_idx << ' ' << n_mo << NL;
    return rcErr;
  }
  auto es = mover->get_endstops();
  if( !es ) {
    OUT << "# Err: no endstop for " << mover_idx << NL;
    return rcErr;
  }

  return go_from_es_nc( mover_idx, mover, es );
}

ReturnCode Machine::go_from_es_nc( unsigned mover_idx, StepMover *mover, EndStop *es )
{
  es->read();
  if( es->is_minus_go() ) {
    OUT << "# Err: not on endstop " << mover_idx << NL;
    return rcErr;
  }
  mover->set_es_mode( StepMover::EndstopMode::From );

  xfloat coo[n_motors];
  std::ranges::fill( coo, 0 );

  bounded_move = true;
  coo[mover_idx] = mover->get_es_find_l();
  auto rc = move_line( coo, fe_g1 * mover->get_k_slow() );
  mover->set_es_mode( StepMover::EndstopMode::Dir );
  return rc;
}

ReturnCode Machine::go_to_es_nc( unsigned mover_idx, StepMover *mover, EndStop *es )
{
  es->read();
  if( es->is_any_stop() ) {
    OUT << "# Err: not clear " << mover_idx << NL;
    return rcErr;
  }
  mover->set_es_mode( StepMover::EndstopMode::Dir );

  xfloat coo[n_motors];
  std::ranges::fill( coo, 0 );

  bounded_move = true;
  coo[mover_idx] = - mover->get_es_find_l();
  auto rc = move_line( coo, fe_g1 * mover->get_k_slow() );
  return rc;
}

// ---------------- G and M code funcs --------------------------

ReturnCode Machine::g_move_line( const GcodeBlock &gc )
{
  const xfloat meas_scale = inchUnit ? 25.4f : 1.0f;

  xfloat prev_x[n_mo], prm[n_mo];
  for( unsigned i=0; i<n_mo; ++i ) {
    prev_x[i] = movers[i]->get_xf();
  }

  for( unsigned i=0; i<n_mo; ++i ) {
    if( axis_chars[i] == '\0' ) {
      break;
    }
    prm[i] = meas_scale * gc.fpv_or_def( axis_chars[i], prev_x[i] / meas_scale );
  }

  if( !relmove ) {
    for( unsigned i=0; i<n_mo; ++i ) {
      prm[i] -= prev_x[i];
    }
  }

  int g = gc.ipv_or_def( 'G', 1 ); // really no check for other: may be used as fallback
  bool g1 { g != 0 };

  xfloat fe_mmm = g1 ? fe_g1 : fe_g0; // TODO: split: 2 different functions
  bounded_move = g1;

  // TODO: comment after after debug
  OUT << "# G" << (g1?'1':'0') << " ( ";
  for( auto xx: prm ) {
    OUT << xx << ' ';
  }
  OUT << " ); fe= "<< fe_mmm << NL;

  ReturnCode rc = move_line( prm, fe_mmm );

  OUT << "#  G0G1 rc= "<< rc << " break_flag= " << break_flag << NL;

  return rc;
}

ReturnCode Machine::g_move_circle( const GcodeBlock &gc )
{
  const xfloat meas_scale = inchUnit ? 25.4f : 1.0f;
  bounded_move = true;

  xfloat prm[13]; // some of them is unused late, but for common

  xfloat &r_s   { prm[0]  };
  xfloat &alp_s { prm[1]  };
  xfloat &r_e   { prm[2]  };
  xfloat &alp_e { prm[3]  };
  xfloat &cv    { prm[4]  };
  xfloat &z_e   { prm[5]  };
  xfloat &e_e   { prm[6]  };
  xfloat &nt    { prm[7]  };
  xfloat &x_e   { prm[8]  };
  xfloat &y_e   { prm[9]  };
  xfloat &x_r   { prm[10] };
  xfloat &y_r   { prm[11] };
  xfloat &r_1   { prm[12] };
  //


  xfloat prev_x[n_mo];
  for( unsigned i=0; i<n_mo; ++i ) {
    prev_x[i] = movers[i]->get_xf();
  }

  // TODO: data-control
  x_e = meas_scale * gc.fpv_or_def( 'X', prev_x[0] / meas_scale );
  y_e = meas_scale * gc.fpv_or_def( 'Y', prev_x[1] / meas_scale );
  z_e = meas_scale * gc.fpv_or_def( 'Z', prev_x[2] / meas_scale );
  e_e = meas_scale * gc.fpv_or_def( 'E', prev_x[3] / meas_scale );
  x_r = meas_scale * gc.fpv_or_def( 'I', NAN );
  y_r = meas_scale * gc.fpv_or_def( 'J', NAN );
  r_1 = meas_scale * gc.fpv_or_def( 'R', NAN );
  nt  = int( gc.fpv_or_def( 'L', 0 ) );
  cv  = gc.ipv_or_def( 'G', 2 ) == 2;

  if( !relmove ) {
    x_e -= prev_x[0]; y_e -= prev_x[1]; z_e -= prev_x[2]; e_e -= prev_x[3];
  }
  // here only relative value

  bool full_circ = fabsxf( x_e ) < near_l && fabsxf( y_e ) < near_l;
  bool ij_mode = isfinite( x_r ) && isfinite( y_r );
  bool r_mode = !ij_mode && isfinite( r_1 );

  if( ! ij_mode && ! r_mode ) {
    std_out << "# Nor I,J nor R defined" << NL;
    return g_move_line( gc );
  }

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

  alp_s = atan2xf(        -y_r  ,        -x_r   );
  alp_e = atan2xf( ( y_e - y_r ), ( x_e - x_r ) );

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

  if( fabsxf( alp_e - alp_s ) < alp_min  ) {
    std_out << "# err: bad d_alp, fallback" << NL;
    return g_move_line( gc );
  }

  xfloat fe_mmm = fe_g1;

  // TODO: comment after after debug
  OUT << "# G" << (cv?'2':'3') << " ( ";
  for( auto xx: prm ) {
    OUT << xx << ' ';
  }
  OUT << " ); fe= "<< fe_mmm << NL;

  auto rc = move_circ( prm, fe_mmm );

  OUT << "#  G2G3 rc= "<< rc << " break_flag= " << break_flag << NL;
  return rc;
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

ReturnCode Machine::g_wait( const GcodeBlock &gc )
{
  xfloat w = 1000 * gc.fpv_or_def( 'P', 0 );
  w += gc.fpv_or_def( 'S', 1 );
  OUT << "# wait " << w << NL;
  delay_ms_brk( (uint32_t)w );
  return ReturnCode::rcOk;
}

ReturnCode Machine::g_set_plane( const GcodeBlock &gc ) // G17
{
  return ReturnCode::rcOk; // the only plane
}

ReturnCode Machine::g_set_unit_inch( const GcodeBlock &gc ) // G20
{
  inchUnit = true;
  return ReturnCode::rcOk;
}

ReturnCode Machine::g_set_unit_mm( const GcodeBlock &gc ) // G21
{
  inchUnit = false;
  return ReturnCode::rcOk;
}

ReturnCode Machine::g_home( const GcodeBlock &gc ) // G28
{
  unsigned bit_s { 0 };
  for( unsigned i = 0, bm = 1; i<n_mo; ++i, bm<<=1 ) {
    char c = axis_chars[i];
    if( c == '\0' ) {
      break;
    }
    if( gc.is_set( c ) ) {
      bit_s |= bm;
    }
  }

  return go_home( bit_s );
}

ReturnCode Machine::g_off_compens( const GcodeBlock &gc ) // G40 - X
{
  OUT << "#  G40 unsupported  " << NL;
  return ReturnCode::rcOk; // for now
}

ReturnCode Machine::g_set_absmove( const GcodeBlock &gc ) // G90
{
  relmove = false;
  return ReturnCode::rcOk;
}

ReturnCode Machine::g_set_relmove( const GcodeBlock &gc ) // G91
{
  relmove = true;
  return ReturnCode::rcOk;
}

ReturnCode Machine::g_set_origin( const GcodeBlock &gc ) // G92
{
  bool none_set { true };

  for( char c : axis_chars ) {
    if( c == '\0' ) {
      break;
    }
    if( gc.is_set( c ) ) {
      none_set = false;
      break;
    }
  }

  unsigned i {0};
  for( char c : axis_chars ) {
    if( c == '\0' ) {
      break;
    }
    if( i > movers.size() ) {
      break;
    }
    if( gc.is_set( c ) || none_set ) {
      xfloat v = gc.fpv_or_def( c, 0 );
      movers[i]->set_xf( v );
    }
    ++i;
  }

  return ReturnCode::rcOk;
}

ReturnCode Machine::m_end0( const GcodeBlock &gc )          // M0
{
  pwm_set( 0, 0 );
  motors_off();
  return ReturnCode::rcEnd;
}

ReturnCode Machine::m_pause( const GcodeBlock &gc )         // M1
{
  // TODO: pause for what?
  OUT << "# pause?" << NL;
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_end( const GcodeBlock &gc )           // M2
{
  pwm_set( 0, 0 );
  motors_off();
  return ReturnCode::rcEnd;
}

ReturnCode Machine::m_set_spin( const GcodeBlock &gc )      // M3, M4
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
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_spin_off( const GcodeBlock &gc )      // M5
{
  OUT << "# M5 " << NL;
  pwm_off( 0 );
  spinOn = false;
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_out_where( const GcodeBlock &gc )     // M114
{
  xfloat k_unit = 1.0f / ( inchUnit ? 25.4f : 1.0f );

  for( auto pm: movers ) {
    OUT << ' ' << pm->get_xf() / k_unit;
  }
  OUT << NL "# F= " << fe_g1 << " S= " << spin << " / " << spin100
      << " n_mo= " << n_mo << ' ' << movers.size() << ' ' << endstops2str_read() << NL;

  return ReturnCode::rcOk;
}

ReturnCode Machine::m_out_str( const GcodeBlock &gc )       // M117
{
  OUT << "# M117" << NL;
  OUT << gc.get_str0() << NL;
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_set_feed_scale( const GcodeBlock &gc )// M220
{
  fe_scale = gc.fpv_or_def( 'S', fe_scale );
  OUT << "# M220 feed scale " << fe_scale << NL;
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_set_spin_scale( const GcodeBlock &gc )// M221
{
  spin100  = gc.fpv_or_def( 'S', spin100 );
  spin_max = gc.fpv_or_def( 'U', spin_max );
  OUT << "# M221 spindle scale " << spin100 << ' ' << spin_max << NL;
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_out_mode( const GcodeBlock &gc )      // M450
{
  static const char* const mode_names[] = { "FFF", "Laser", "CNC", "??3", "??4", "??5" };
  OUT << "PrinterMode:" << mode_names[mode] << NL;
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_set_mode_fff( const GcodeBlock &gc )  // M451
{
  OUT << "# M451 " << NL;
  set_mode( Machine::modeFFF );
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_set_mode_laser( const GcodeBlock &gc )// M452
{
  OUT << "# M452 " << NL;
  set_mode( Machine::modeLaser );
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_set_mode_cnc( const GcodeBlock &gc )  // M453
{
  OUT << "# M453 " << NL;
  set_mode( Machine::modeCNC );
  return ReturnCode::rcOk;
}

// local codes
ReturnCode Machine::m_list_vars( const GcodeBlock &gc )     // M995
{
  out_vals();
  return ReturnCode::rcOk;
}

ReturnCode Machine::m_set_var(   const GcodeBlock &gc )     // M996 name=S, value = V
{
  return set_val( gc.get_str0(), gc.fpv( 'V' ) );
}

ReturnCode Machine::m_get_var(   const GcodeBlock &gc )     // M997 name=S
{
  auto v = get_val( gc.get_str0() );
  OUT << "# " << gc.get_str0() << " = ";
  if ( isfinite( v ) )  {
    OUT << v << NL;
    return ReturnCode::rcOk;
  }
  OUT << "NAN" << NL;
  return ReturnCode::rcErr;
}


ReturnCode Machine::set_val( const char *name, xfloat v )
{
  if( !name ) {
    return ReturnCode::rcErr;
  }
  for( const auto &vi: var_info ) {
    if( strcmp( name, vi.name ) == 0 ) {
      if( vi.fptr ) {
        this->*(vi.fptr) = v;
        return ReturnCode::rcOk;
      } else if( vi.iptr ) {
        this->*(vi.iptr) = (int)v;
        return ReturnCode::rcOk;
      }
      return ReturnCode::rcErr;
    }
  }
  return ReturnCode::rcErr;
}

xfloat Machine::get_val( const char *name ) const
{
  if( !name ) {
    return NAN;
  }
  for( const auto &vi: var_info ) {
    if( strcmp( name, vi.name ) == 0 ) {
      if( vi.fptr ) {
        return this->*(vi.fptr);
      } else if( vi.iptr ) {
        return (int)( this->*(vi.iptr) );
      }
      return NAN;
    }
  }
  return NAN;
}

void   Machine::out_vals() const
{
  for( const auto &vi: var_info ) {
    OUT << "# " <<  vi.name << " = ";
    if( vi.fptr ) {
      OUT << (this->*(vi.fptr)) ;
    } else if( vi.iptr ) {
      OUT << (int)( this->*(vi.iptr) );
    }
    OUT << NL;
  }
}

ReturnCode Machine::call_mg( const GcodeBlock &cb )
{
  cb.dump(); // debug
  auto is_g = cb.is_set('G');
  auto is_m = cb.is_set('M');
  if( is_g && is_m ) {
    OUT << "#  MS error: M and G" << NL;
    return ReturnCode::rcErr; // TODO: err_val: both commands
  }

  auto mach_rc = prep_fun( cb );
  if( mach_rc >= ReturnCode::rcErr ) {
    return mach_rc;
  }

  char chfun  = is_m ? 'M' : 'G';
  int code    = ( is_m ? 1000000 : 0 ) + 1000 * cb.fpv_or_def( chfun, -10000 );

  auto f = std::find_if( mg_funcs, mg_funcs + mg_funcs_sz, [code]( auto el ) { return el.num == code; } );
  if( f == mg_funcs + mg_funcs_sz ) {
    OUT << "# warn: unsupported " << chfun << ' ' << code << NL;
    return ReturnCode::rcOk;
  }

  auto fun = f->fun;
  auto rc = (this->*fun)( cb );

  if( debug > 0 ) {
    OUT << "# debug: xf:";
    for( auto pm: movers ) {
      OUT << ' ' << pm->get_xf();
    }
    OUT << NL "# debug: xi:";
    for( auto pm: movers ) {
      OUT << ' ' << pm->get_x();
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

ReturnCode Machine::prep_fun(  const GcodeBlock &gc )
{
  if( gc.is_set('M') ) { // special values for M commands
    // OUT << 'M' << NL;
    return ReturnCode::rcOk;
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
  return ReturnCode::rcOk;
}

// --------------------------- PWM ----------------------------------------


ReturnCode pwm_set( unsigned idx, xfloat v )
{
  if( idx >= n_tim_pwm ) {
    return rcErr;
  }
  auto ti = pwm_tims[idx]->Instance;
  uint32_t arr = ti->ARR;
  uint32_t ccr = (uint32_t)( arr * v / 100.0f );
  ti->CCR1 = ccr;
  ti->CNT  = 0;
  return rcOk;
}

ReturnCode pwm_off( unsigned idx )
{
  if( idx >= n_tim_pwm ) {
    return rcErr;
  }
  auto ti = pwm_tims[idx]->Instance;
  ti->CCR1 = 0;
  ti->CNT  = 0;
  //  HAL_TIM_PWM_Stop( &htim3, TIM_CHANNEL_1 );
  return rcOk;
}

ReturnCode pwm_off_all()
{
  for( auto t : pwm_tims ) {
    if( t && t->Instance ) {
      t->Instance->CCR1 = 0;
      t->Instance->CNT  = 0;
    }
  }
  return rcOk;
}


int cmd_pwm( int argc, const char * const * argv )
{
  int    ch  = arg2long_d(  1, argc, argv, 0, 0, n_tim_pwm ); // including limit for ALL_OFF
  xfloat pwr = arg2xfloat_d( 2, argc, argv, 0, 0.0f, 100.0f );

  if( (unsigned)ch >= n_tim_pwm ) {
    pwm_off_all();
    std_out << "# PWM: stop all" << NL;
    return 0;
  }

  std_out << "# PWM: ch: " << ch << " power: " << pwr << NL;
  auto rc  = pwm_set( ch, pwr );
  // tim_print_cfg( pwm_tims[ch]->Instance );
  return (int)rc; // rcOk = 0 = cmd_ok
}

// ------------------------------ EXTI handlers -------------------

void HAL_GPIO_EXTI_Callback( uint16_t pin_bit )
{
  ++UVAR('i'); UVAR('b') = pin_bit;
  leds[1].toggle();
  // break_flag = 0x000F0000 + pin_bit;
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

// ------------------------------------------------------------

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

// ----------------------------------------  ------------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

