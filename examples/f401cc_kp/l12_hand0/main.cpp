#include <cerrno>
#include <climits>
#include <algorithm>
#include <numbers>
#include <cmath>
#include <array>
#include <ranges>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_cpptypes.h>
#include <oxc_atleave.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
#include <oxc_atleave.h>
#include <oxc_outstr.h>
#include <oxc_as5600.h>

#include "main.h"


namespace ranges = std::ranges;
namespace views = std::views;
using std::size_t;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

int debug {0};
int dry_run {0};
int dis_movers {0};
int def_tp {3};
int angle_over { 20 };

auto out_q_fmt = [](xfloat x) { return FltFmt(x, cvtff_fix,8,4); };

PinsOut ledsx( LEDSX_GPIO, LEDSX_START, LEDSX_N );
PinsIn  pin_stop( BTN_STOP_GPIO, BTN_STOP_PIN, 1, GpioRegs::Pull::up );


TIM_HandleTypeDef tim_lwm_h;

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );

int adc_n {100};
volatile int adc_dma_end {0};

// part funcs
static constexpr float pi_f = std::numbers::pi_v<float>;
static constexpr float pi_half_f = pi_f / 2;

float pafun_one( float x )
{
  return pafun_lim( x );
}

float pafun_poly2_ss( float x )
{
  x = pafun_lim( x );
  return x*x;
}

float pafun_poly2_se( float x )
{
  x = pafun_lim( x );
  return x * ( 2 - x );
}

float pafun_poly3_sb( float x )
{
  x = pafun_lim( x );
  return -2 * x*x*x + 3*x*x;
}

float pafun_trig_ss(  float x )
{
  x = pafun_lim( x );
  return 1 - cosf( x * pi_half_f );
}

float pafun_trig_se(  float x )
{
  x = pafun_lim( x );
  return sinf( x * pi_half_f );
}

float pafun_trig_sb(  float x )
{
  x = pafun_lim( x );
  return 0.5f * ( 1 - cosf( pi_f * x ) );
}


const PartFunInfo part_fun_info[] = {
  { pafun_one,      1.0f },           // 0
  { pafun_poly2_ss, 0.5f },           // 1
  { pafun_poly2_se, 0.5f },           // 2
  { pafun_poly3_sb, 0.6f },           // 3
  { pafun_trig_ss,  1/pi_half_f },    // 4
  { pafun_trig_se,  1/pi_half_f },    // 5
  { pafun_trig_sb,  1/pi_half_f },    // 6
  { pafun_one,      1.0f } // protect // 7
};
constexpr auto part_fun_n { std::size( part_fun_info) };

OutStream& operator<<( OutStream &os, const MovePartCoord &rhs )
{
  os << "{ " << out_q_fmt(rhs.q_e) << " , " << rhs.tp << " }";
  return os;
}

OutStream& operator<<( OutStream &os, const MovePart &rhs )
{
  os << out_q_fmt(rhs.k_v) << ' ';
  for( const auto& m : rhs.mpc ) {
    os << m << ' ';
  }
  return os;
}

void MovePart::from_coords( std::span<const CoordInfo> coos, unsigned tp )
{
  for( std::tuple<const CoordInfo &, MovePartCoord &> i : ranges::views::zip( coos, mpc ) ) {
    std::get<1>(i).q_e = std::get<0>(i).q_cur;
    std::get<1>(i).tp = tp;
  }
}

// ----------- sensors ----------------

SensorAS5600 sens_enc( ang_sens );
SensorAdc sens_adc( ADC1_NCH );
SensorFakeMover sens_grip( mover_grip );

std::array<Sensor*,3> sensors { &sens_adc, &sens_enc, &sens_grip };

// ------------------------------- Movers -----------------------------------

MoverServoCont mover_base(   0.01f,     1.0f, TIM_LWM->CCR1, TIM_LWM->ARR, &coords[0].q_cur );
MoverServo     mover_p1(   -10.0f,   2500.0f, TIM_LWM->CCR2, TIM_LWM->ARR, &coords[1].q_cur );
MoverServo     mover_p2(    10.0f,   2850.0f, TIM_LWM->CCR3, TIM_LWM->ARR, &coords[2].q_cur );
MoverServo     mover_grip(  10.0f,    500.0f, TIM_LWM->CCR4, TIM_LWM->ARR, &coords[3].q_cur );

std::array<Mover*,movers_n> movers { &mover_base, &mover_p1, &mover_p2, &mover_grip };


// ------------------------   end Movers

// ------------------------------- Coords -----------------------------------



CoordInfo coords[coords_n] {
//   q_min   q_max  vt_max   sens   sens_ch     mo         q_cur
  { -90.0f,  90.0f,  80.0f,  &sens_enc,  0,  &mover_base,   0.0f }, // rotate
  {  45.0f,  95.0f,  90.0f,  &sens_adc,  0,  &mover_p1,    80.0f }, // arm1
  {-135.0f, -90.0f,  90.0f,  &sens_adc,  1,  &mover_p2,   -70.0f }, // arm2
  {   0.0f,  90.0f, 120.0f, &sens_grip,  0,  &mover_grip,  30.0f }, // grip
};

// ------------------------   default sequence

const MovePart mp_seq0[] {
  { { {   0.0f, 3 }, {   90.0f, 3 }, {   -130.0f, 3 }, {   50.0f, 3 }  },  0.5f },
  { { { -20.0f, 3 }, {   50.0f, 3 }, {   -130.0f, 3 }, {   50.0f, 3 }  },  0.5f },
  { { { -20.0f, 3 }, {   50.0f, 3 }, {   -130.0f, 3 }, {    5.0f, 3 }  },  0.5f },
  { { { -80.0f, 3 }, {   60.0f, 3 }, {   -110.0f, 3 }, {    5.0f, 3 }  },  0.5f },
  { { { -80.0f, 3 }, {   60.0f, 3 }, {   -110.0f, 3 }, {   50.0f, 3 }  },  0.5f },
  { { {   0.0f, 3 }, {   90.0f, 3 }, {   -130.0f, 3 }, {   50.0f, 3 }  },  0.5f },
  { { {  20.0f, 3 }, {   50.0f, 3 }, {   -130.0f, 3 }, {   50.0f, 3 }  },  0.5f },
  { { {  20.0f, 3 }, {   50.0f, 3 }, {   -130.0f, 3 }, {    5.0f, 3 }  },  0.5f },
  { { {  80.0f, 3 }, {   60.0f, 3 }, {   -110.0f, 3 }, {    5.0f, 3 }  },  0.5f },
  { { {  80.0f, 3 }, {   60.0f, 3 }, {   -110.0f, 3 }, {   50.0f, 3 }  },  0.5f },
  { { {   0.0f, 3 }, {   90.0f, 3 }, {   -135.0f, 3 }, {   10.0f, 3 }  },  0.5f }
};

std::array<MovePart,mp_stored_n> mp_stored;
MovePart mp_last;
MovePart mp_old;
std::array<MovePart,mp_seq1_n> mp_seq1;
size_t mp_seq1_sz { 0 };

// ------------------------   commands

const char* common_help_string = "hand0 " __DATE__ " " __TIME__ NL;

// --- local commands;
DCL_CMD_REG( test0,       'T', " [val] [ch] [k_v] - test move 1 ch" );
DCL_CMD_REG( stop,        'P', " - stop pwm" );
DCL_CMD_REG( mtest,       'M', " [set_zero] [aux] - test AS5600" );
DCL_CMD_REG( mcoord,      'C', " [store_idx] [n_adc] - measure and store coords" );
DCL_CMD_REG( go,          'G', " k_v q_0 q_1 q_2 q_3 tp_0 tp_1 tp_2 tp_3 - go " );
DCL_CMD_REG( add_mp,      'A', " k_v q_0 q_1 q_2 q_3 tp_0 tp_1 tp_2 tp_3 - add MovePoint " );
DCL_CMD_REG( edit_mp,     'E', " n k_v q_0 q_1 q_2 q_3 tp_0 tp_1 tp_2 tp_3 - edit MovePoint " );
DCL_CMD_REG( add_stored,  'S', " [store_idx] [kv] [q3] - add stored MovePoint " );
DCL_CMD_REG( add_last,    'L', " [kv] - add last MovePoint " );
DCL_CMD_REG( del_mp,     '\0', " [n] - delete def=last MovePoint " );
DCL_CMD_REG( clear_mp,   '\0', " - delete all MovePoints " );
DCL_CMD_REG( run,         'R', " [seq_num] - run sequence " );
DCL_CMD_REG( back,        'B', " [kv] - go to last point " );
DCL_CMD_REG( go_stored,   'Q', " [n] [kv] - go to stored  point " );
DCL_CMD_REG( pulse,       'U', " ch t_on dt off - test pulse " );
DCL_CMD_REG( out_moves,   'O', " [seq_num]  - output moves sequence " );
DCL_CMD_REG( calibr,     '\0', " ch(1-2) - calibrate channel sensor " );



// ---------------------- named data --------------------------------------


#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_IOBJ_TD(x) constexpr NamedInt   ob_##x { #x, &td.x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }
#define ADD_FOBJ_TD(x) constexpr NamedFloat ob_##x { #x, &td.x }

// ADD_IOBJ_TD( n_total );
// ADD_FOBJ_TD( d_wire  );
ADD_IOBJ   ( debug   );
ADD_IOBJ   ( dry_run   );
ADD_IOBJ   ( dis_movers   );
ADD_IOBJ   ( def_tp   );
ADD_IOBJ   ( angle_over   );
ADD_IOBJ   ( adc_n   );

#undef ADD_IOBJ
#undef ADD_IOBJ_TD


constexpr const NamedObj *const objs_info[] = {
  & ob_debug,
  & ob_dry_run,
  & ob_dis_movers,
  & ob_def_tp,
  & ob_angle_over,
  & ob_adc_n,
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
}

void on_sigint( int /* c */ )
{
  tim_lwm_stop();
  break_flag = 1;
  ledsx[1].set();
}

int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR_t =    50;
  UVAR_n =    20;
  UVAR_k =  1000; // axis 0 rotate test coeff * 1000

  ledsx.initHW();
  ledsx.reset( 0xFF );
  pin_stop.initHW();

  UVAR_v = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens;

  MX_DMA_Init();
  if( ! MX_ADC1_Init() ) {
    std_out << "Err: ADC init"  NL;
    die4led( 3 );
  }

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  if( ! tim_lwm_cfg() ) {
    std_out << "Err: timer LWM init"  NL;
    die4led( 2 );
  }

  init_EXTI();

  ranges::for_each( movers,  [](auto pm) { pm->init(); } );
  ranges::for_each( sensors, [](auto ps) { ps->init(); } );
  sens_enc.set_zero_val( 2520 ); // mech param: init config?
  mover_base.set_lwm_times( 1300, 1700 );

  tim_lwm_start();
  measure_store_coords( adc_n );

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  dev_console.setOnSigInt( on_sigint );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}



void init_EXTI()
{
  BTN_STOP_GPIO.setEXTI( BTN_STOP_PIN, BTN_STOP_EXTI_DIR );
  HAL_NVIC_SetPriority( BTN_STOP_IRQ_N, BTN_STOP_IRQ_PRTY, 0 );
  HAL_NVIC_EnableIRQ(   BTN_STOP_IRQ_N );
}

int bad_coord_idx() // -1 = Ok
{
  for( const auto [ i, co ] : views::enumerate(coords) ) {
    const auto c0 = co.q_cur;
    if( c0 < co.q_min - angle_over || c0 > co.q_max + angle_over ) { // TODO: param?
      return (int)i;
    }
  }
  return -1;
}

bool is_good_coords( bool do_stop, bool do_print, bool do_measure  )
{
  if( do_measure && !measure_store_coords( adc_n ) ) {
    if( do_print ) {
      std_out << "# Err sens : " << NL;
    }
    return false;
  }
  int bad_idx = bad_coord_idx();
  if( bad_idx < 0 ) {
    return true;
  };
  if( do_stop ) {
    tim_lwm_stop();
  }
  if( do_print ) {
    std_out << "# Err: bad coord " << bad_idx << ' ' << coords[bad_idx].q_cur << NL;
  }
  return false;
}

bool is_mover_disabled( unsigned ch, bool do_print )
{
  if( (1<<ch) & dis_movers ) {
    if( do_print ) {
      std_out << "# Warn: mover " << ch << " is disabled " << dis_movers << NL;
    }
    return true;
  }
  return false;
}

int cmd_test0( int argc, const char * const * argv )
{
  if( debug > 0 ) {
    tim_print_cfg( TIM_LWM );
  }

  return 0;
}


int cmd_stop( int argc, const char * const * argv )
{
  tim_lwm_stop();
  return 0;
}

int cmd_go( int argc, const char * const * argv )
{
  static_assert( coords_n <= MovePart::n_max );
  MovePart mp;
  cmd2MovePart( argc, argv, 1, mp );
  mp_last = mp;

  std_out << "# Go:  " << mp << NL;

  tim_lwm_start();
  int rc = process_movepart( mp, 1.0f );
  return rc;
}

int cmd_add_mp( int argc, const char * const * argv )
{
  if( is_overflow_seq() ) {
    return 1;
  }

  MovePart &mp { mp_seq1[mp_seq1_sz] };
  cmd2MovePart( argc, argv, 1, mp );

  std_out << "# Added:  " << mp << NL;
  ++mp_seq1_sz;
  return 0;
}

int cmd_edit_mp( int argc, const char * const * argv )
{
  const unsigned  n = arg2long_d(  1, argc, argv, 1, 0, mp_seq1_sz-1 );

  MovePart &mp { mp_seq1[n] };
  cmd2MovePart( argc, argv, 2, mp );

  std_out << "# Edited:  " << mp << NL;
  return 0;
}

int cmd_clear_mp( int argc, const char * const * argv )
{
  mp_seq1_sz = 0;
  return 0;
}

int cmd2MovePart( int argc, const char * const * argv, int start_idx, MovePart &mp )
{
  mp.init();

  mp.k_v = arg2float_d( start_idx, argc, argv, MovePart::kv_def, MovePart::kv_min, MovePart::kv_max );

  for( auto [i,m]: views::enumerate(mp.mpc) ) {
    m.q_e = arg2float_d( i+start_idx+1,          argc, argv, coords[i].q_cur, coords[i].q_min, coords[i].q_max );
    m.tp  = arg2long_d(  i+start_idx+1+coords_n, argc, argv, def_tp, 0, part_fun_n-1 );
  }

  return 0;
}

int cmd_run( int argc, const char * const * argv )
{
  int mps_idx   = arg2long_d(   1, argc, argv,    0,    0,    1 );
  float kkv     = arg2float_d(  2, argc, argv, 1.0f, 0.01f, 5.0f );
  int i_start   = arg2long_d(   3, argc, argv,    0,    0, 10000 );
  int i_end     = arg2long_d(   4, argc, argv, 10000,   0, 10000 );
  std_out << "# run: " << mps_idx << NL;
  auto seq = ( mps_idx != 0 ) ?
    ( std::span<const MovePart> (mp_seq0) ) :
    ( std::span<const MovePart> (mp_seq1.begin(), mp_seq1_sz) );
  tim_lwm_start();
  int rc = run_moveparts( seq, kkv, i_start, i_end );
  return rc;
}

int cmd_back( int argc, const char * const * argv )
{
  float kkv     = arg2float_d(  1, argc, argv, 1.0f, 0.01f, 5.0f );
  std_out << "# back: " << mp_stored[mp_stored_n-1] << NL;
  tim_lwm_start();
  int rc = process_movepart( mp_stored[mp_stored_n-1], kkv );
  return rc;
}

int cmd_go_stored( int argc, const char * const * argv )
{
  int stored_idx = arg2float_d(  1, argc, argv,    0,     0, mp_stored_n-1 );
  float kkv      = arg2float_d(  2, argc, argv, 1.0f, 0.01f,          5.0f );
  std_out << "# go stored: " << mp_stored[stored_idx] << NL;
  tim_lwm_start();
  int rc = process_movepart( mp_stored[stored_idx], kkv );
  return rc;
}

// just for debug, remove
int cmd_pulse( int argc, const char * const * argv )
{
  int      ch   = arg2long_d(  1, argc, argv,    1,    0, std::size(coords)-1 );
  uint32_t t_on = arg2long_d(  2, argc, argv, 1500,  400, 2600  );
  uint32_t dt   = arg2long_d(  3, argc, argv,  500,   10, 5000  );
  uint32_t keep = arg2long_d(  4, argc, argv,    1,    0,    1  );

  std_out << "# pulse: ch= " << ch << " t_on= " << t_on << " dt= " << dt << NL;

  if( is_mover_disabled( ch, true ) ) {
    return 1;
  }

  measure_store_coords( adc_n );
  std_out << "# start: ";
  out_coords( true );

  auto& co = coords[ch];
  if( co.mo == nullptr ) {
    return 10;
  }
  auto &mo = *co.mo;

  mo.pre_run( co.q_cur, 0, 1000 ); // fake values here
  delay_ms( 50 );
  mo.setCtrlVal( t_on );
  tim_lwm_start();

  ledsx[3].set();

  const uint32_t t_step { uint32_t(UVAR_t) };
  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0, tc00 = tm0;

  break_flag = 0;
  for( uint32_t i=0; i<dt && !break_flag; i += t_step ) {

    if( ! is_good_coords( true, true, true )  ) {
      break;
    }

    uint32_t  tcc = HAL_GetTick();
    std_out << FmtInt(i,4) << ' ' << FmtInt( tcc - tc00, 6 ) << ' ';

    out_coords( true );

    delay_ms_until_brk( &tc0, t_step );
  }
  ledsx[3].reset();

  mo.post_run();
  if( ! keep || break_flag ) {
    tim_lwm_stop();
  }

  measure_store_coords( adc_n );
  std_out << "# after: ";
  out_coords( false );
  out_coords_int( true );
  return 0;
}

int cmd_out_moves( int argc, const char * const * argv )
{
  int mps_idx   = arg2long_d(   1, argc, argv,    0,    0,    1 );
  std_out << "# Moves " << mps_idx << NL;
  auto seq = ( mps_idx != 0 ) ?
    ( std::span<const MovePart> (mp_seq0) ) :
    ( std::span<const MovePart> (mp_seq1.begin(), mp_seq1_sz) );
  for( const auto[ i,m ]: views::enumerate(seq) ) {
    std_out << i << ' ' << m << NL;
  }
  return 0;
}

int cmd_del_mp( int argc, const char * const * argv )
{
  if( mp_seq1_sz < 1 ) {
    return 1;
  }
  size_t mp_idx   = arg2long_d(   1, argc, argv,    mp_seq1_sz-1,  0,  mp_seq1_sz-1 );
  std::shift_left( mp_seq1.begin()+mp_idx, mp_seq1.begin()+mp_seq1_sz, 1 );
  --mp_seq1_sz;
  return 0;
}

bool is_overflow_seq()
{
  if( mp_seq1_sz >= mp_seq1_n ) {
    std_out << "# Error: overflow  " << mp_seq1_sz << ' ' << mp_seq1_n << NL;
    return true;
  }
  return false;
}


int cmd_add_stored( int argc, const char * const * argv )
{
  if( is_overflow_seq() ) {
    return 1;
  }
  size_t mp_idx = arg2long_d( 1, argc, argv, 0, 0, mp_stored.size() - 1 );
  float k_v = arg2float_d( 2, argc, argv, MovePart::kv_def, MovePart::kv_min, MovePart::kv_max );
  float q3  = arg2float_d( 3, argc, argv, mp_stored[mp_idx].mpc[3].q_e, 0.0f, 90.0f );

  MovePart &mp { mp_seq1[mp_seq1_sz] };
  mp     = mp_stored[mp_idx];
  mp.k_v = k_v;
  mp.mpc[3].q_e = q3;

  std_out << "# Added:  " << mp << NL;
  ++mp_seq1_sz;
  return 0;
}

int cmd_add_last( int argc, const char * const * argv )
{
  if( is_overflow_seq() ) {
    return 1;
  }
  float k_v = arg2float_d( 1, argc, argv, MovePart::kv_def, MovePart::kv_min, MovePart::kv_max );

  MovePart &mp { mp_seq1[mp_seq1_sz] };
  mp     = mp_last;
  mp.k_v = k_v;

  std_out << "# Added:  " << mp << NL;
  ++mp_seq1_sz;
  return 0;
}


int cmd_calibr( int argc, const char * const * argv )
{
  int  ch     = arg2long_d(  1, argc, argv,    1,    0, std::size(coords)-1 );
  int  store  = arg2long_d(  2, argc, argv,    0,    0, 1 );
  std_out << "# calibr ch: "<< ch << NL;

  if( ch < 1 || ch > 2 ) {
    std_out << "# Warn: calibration is not available for channel for now: " << ch << NL;
    return 1;
  }

  RestoreAtLeave keep_movers { dis_movers,  ~ (1<<ch) };// disable all except selected

  measure_store_coords( adc_n );
  auto &co  = coords[ch];

  MovePart mp;
  const auto q_min_given = co.q_min + 5.0f; // TODO: calibrate param
  const auto q_max_given = co.q_max - 5.0f;
  mp.init();
  for( size_t i=0; i<std::size(coords); ++i ) { // prevent other axis move, among disabling movers
    mp.mpc[i].q_e = coords[i].q_cur;
  }

  mp.mpc[ch].q_e = q_min_given;
  mp.k_v = 0.2f;  // TODO: param

  int rc = process_movepart( mp );
  if( rc != 0 || break_flag ) {
    std_out << "# Err: fail to find start. ch: " << ch << ' ' << q_min_given << NL;
    return 2;
  }
  // TODO: use structure
  const unsigned adc_ch { (ch == 1) ? 0u : 1u };
  // TODO: structure
  const auto q_min_get { co.q_cur };
  const auto q_min_raw  { sens_adc.getInt( adc_ch ) };
  std_out << "# min " NL;
  std_out << "# given: " << q_min_given << " get: " << q_min_get
          <<  " raw: " << q_min_raw << NL;
  // TODO: check max delta given-get, but not too strict (or 'force' flag)

  delay_ms( 500 );
  mp.mpc[ch].q_e = q_max_given;
  rc = process_movepart( mp );
  if( rc != 0  || break_flag ) {
    std_out << "# Err: fail to find end. ch: " << ch << ' ' << q_max_given << NL;
    return 2;
  }

  const auto q_max_get { co.q_cur };
  const auto q_max_raw  { sens_adc.getInt( adc_ch ) }; // TODO: better structure
  std_out << "# max " NL;
  std_out << "# given: " << q_max_given << " get: " << q_max_get
          << " raw: " << q_max_raw << NL;

  const auto    d_given  { q_max_given - q_min_given };
  const int32_t d_raw    { q_max_raw   - q_min_raw };
  std_out << "# delta given: " << d_given << " raw: " << d_raw << NL;
  if( fabsf( d_given ) < 0.1f || abs( d_raw ) < 50 ) {
    std_out << "# Error: low delta"  NL;
    return 1;
  }

  const float k_a { d_given / d_raw };
  const float k_b { q_min_given - k_a * q_min_raw };
  std_out << "# k_a= " << k_a << " k_b= " << k_b << NL;

  if( !store ) {
    return 0;
  }

  const float k_x[2] { k_a,  k_b };
  sens_adc.setCalibr( adc_ch, k_x );

  return 0;
}

int cmd_mtest( int argc, const char * const * argv )
{
  const int  set_pos = arg2long_d( 1, argc, argv, 0, 0, 1 );
  const int  aux     = arg2long_d( 2, argc, argv, 0, INT_MIN, INT_MAX );
  sens_enc.measure( 1 );
  auto alp_i = sens_enc.getInt(0);
  auto alp_v = sens_enc.get( 0 );

  std_out
      << alp_i << ' ' << alp_v << ' ' << aux
      << ' ' << ang_sens.getN_turn() << ' ' << ang_sens.getOldVal() << NL;

  std_out << "=== AGC: " << ang_sens.getAGCSetting() << " cordic: " <<  ang_sens.getCORDICMagnitude()
    << " detect: "  << ang_sens.isMagnetDetected() << " status: " << HexInt8( ang_sens.getStatus() ) << NL;
  if( set_pos ) {
    ang_sens.setStartPosCurr();
  }
  return 0;
}

int cmd_mcoord( int argc, const char * const * argv )
{
  const size_t store_idx = arg2long_d(  1, argc, argv, 0,     0, mp_stored.size()-1 );
  const int n_meas       = arg2long_d(  2, argc, argv, adc_n, 1, 10000 );
  int rc  = measure_store_coords( n_meas );
  out_coords( true );
  if( debug > 0 ) {
    out_coords_int( true );
  }

  if( rc ) {
    mp_stored[store_idx].from_coords( coords, 3 );
  }

  return !rc;
}

void out_coords( bool nl )
{
  for( auto c : coords ) {
    std_out << c.q_cur << ' ';
  }
  if( nl ) {
    std_out << NL;
  }
}

void out_coords_int( bool nl )
{
  for( auto &co : coords ) {
    std_out << co.sens->getInt( co.sens_ch ) << ' ';
  }
  if( nl ) {
    std_out << NL;
  }
}

int measure_store_coords( int nm )
{
  for( auto ps : sensors ) {
    auto rc = ps->measure( nm );
    if( rc < 1 ) {
      return 0;
    }
  }

  for( auto &co : coords ) {
    co.q_cur = co.sens->get( co.sens_ch );
  }

  return 1;
}

int process_movepart( const MovePart &mp, float kkv  )
{
  const uint32_t t_step = UVAR_t;
  float qs_0[coords_n];
  float qs_dlt[coords_n];

  uint32_t nn {0};
  measure_store_coords( adc_n );
  mp_old.from_coords( coords, 3 );
  for( size_t i=0; i < coords_n; ++i ) { // calc max need time in steps
    auto &co = coords[i];
    qs_0[i]    = co.q_cur;
    qs_dlt[i]  = mp.mpc[i].q_e - qs_0[i];
    const float q_adlt { fabsf( qs_dlt[i] ) };
    const float v = kkv * mp.k_v * co.vt_max * part_fun_info[ mp.mpc[i].tp ].kv;

    const uint32_t n = std::clamp( unsigned( q_adlt/( v * t_step * 1e-3f ) ), 1u, 10000u );
    nn = std::max( nn, n );
    std_out << "# plan: n= " << n << " nn= " << nn << " qs_0= " << qs_0[i] << " dlt= " << qs_dlt[i] <<  NL;
  }
  std_out << "# k_v= " << mp.k_v << " nn= " << nn << NL;


  ledsx.reset ( 0x03 );

  for( uint32_t ch=0; auto mo : movers ) {
    if( is_mover_disabled( ch ) ) {
      continue;
    }
    if( !mo || ! mo->pre_run( mp.mpc[ch].q_e, mp.mpc[ch].tp, nn ) ) {
      std_out << "# Err: pre_run" NL;
      return 2;
    }
  }

  std_out << "#  1      2      3       4        5         6    7        8           9          10          11"   NL;
  std_out << "#  i   tick     q_g0     q_g1     q_g2     q_g3 t_on    q_m0        q_m1        q_m2         q_m3" NL;

  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0, tc00 = tm0;

  break_flag = 0;
  for( decltype(+nn) i=0; i<nn && !break_flag; ++i ) {

    ledsx[3].set();

    if( ! is_good_coords( true, true, true )  ) {
      break_flag = 1;
      break;
    }

    uint32_t  tcc = HAL_GetTick();
    std_out << FmtInt(i,4) << ' ' << FmtInt( tcc - tc00, 6 ) << ' ';

    for( size_t mi = 0; mi<coords_n; ++mi ) {

      const float q01f = part_fun_info[ mp.mpc[mi].tp ].f( float(i+1)/nn );
      const float q = qs_0[mi] + qs_dlt[mi] * q01f;
      std_out << out_q_fmt( q ) << ' ';

      if( dry_run || is_mover_disabled( mi ) || movers[mi] == nullptr ) {
        continue;
      }

      movers[mi]->move( q, tcc );
    }

    ledsx[3].reset();

    std_out << dbg_val0 << ' ';

    out_coords( true );

    delay_ms_until_brk( &tc0, t_step );
  }

  for( uint32_t ch=0; auto mo : movers ) {
    if( is_mover_disabled( ch ) ) {
      continue;
    }
    if( !mo || ! mo->post_run() ) {
      std_out << "# Err: post_run" NL;
    }
  }

  measure_store_coords( adc_n );
  std_out << "# end: " << break_flag << ' ';
  out_coords( true );

  return break_flag;
}

int run_moveparts( std::span<const MovePart> mps, float kkv, int i_start, int i_end )
{
  uint32_t tm0 = HAL_GetTick();
  for( const auto [pn, mp] : views::enumerate(mps) ) {
    // _ShowType< decltype(mp) > xType; // -> <const MovePart&>
    uint32_t tc = HAL_GetTick();
    std_out << "## part " << pn << " start, t= " << (tc - tm0) << NL;
    if( pn < i_start || pn > i_end ) {
      continue;
    }
    auto rc = process_movepart( mp, kkv );
    tc = HAL_GetTick();
    std_out << "## part " << pn << " end, t= " << (tc - tm0) << NL;
    if( rc != 0 ) {
      return rc;
    }
  }
  return 0;
}

// -------------------- Timers ----------------------------------------------------

int tim_lwm_cfg()
{
  const uint32_t psc { calc_TIM_psc_for_cnt_freq( TIM_LWM, tim_lwm_psc_freq ) };
  const auto tim_lwm_arr = calc_TIM_arr_for_base_psc( TIM_LWM, psc, tim_lwm_freq );
  UVAR_a = psc;
  UVAR_b = tim_lwm_arr;

  auto &t_h { tim_lwm_h };
  t_h.Instance               = TIM_LWM;
  t_h.Init.Prescaler         = psc;
  t_h.Init.Period            = tim_lwm_arr;
  t_h.Init.ClockDivision     = 0;
  t_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  t_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &t_h ) != HAL_OK ) {
    UVAR_e = 1; // like error
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &t_h, &sClockSourceConfig );

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &t_h, &sMasterConfig ) != HAL_OK ) {
    UVAR_e = 2;
    return 0;
  }

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.Pulse        = 0; // tim_lwm_arr / 2; // TMP to test, need 0;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  for( auto ch : { TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3, TIM_CHANNEL_4 } ) {
    HAL_TIM_PWM_Stop( &t_h, ch );
    if( HAL_TIM_PWM_ConfigChannel( &t_h, &tim_oc_cfg, ch ) != HAL_OK ) {
      UVAR_e = 3000 + ch;
      return 0;
    }
  }
  return 1;
}


void tim_lwm_start()
{
  for( auto ch : { TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3, TIM_CHANNEL_4 } ) {
    HAL_TIM_PWM_Start( &tim_lwm_h, ch );
  }
  ledsx[2].set();
}

void tim_lwm_stop()
{
  for( auto ch : { TIM_CHANNEL_1, TIM_CHANNEL_2,TIM_CHANNEL_3, TIM_CHANNEL_4 } ) {
    HAL_TIM_PWM_Stop( &tim_lwm_h, ch );
  }
  ledsx[2].reset();
}



bool read_sensors()
{
  return true;
}


void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_LWM ) {
    TIM_LWM_EN;
    GpioA.cfgAF_N( TIM_LWM_GPIO_PINS, TIM_LWM_GPIO_AF );
    // HAL_NVIC_SetPriority( TIM_LWM_IRQn, 8, 0 );
    // HAL_NVIC_EnableIRQ( TIM_LWM_IRQn );
    return;
  }


}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_LWM ) {
    TIM_LWM_DIS;
    GpioA.cfgIn_N( TIM_LWM_GPIO_PINS );
    // HAL_NVIC_DisableIRQ( TIM_LWM_IRQn );
    return;
  }
}

// void TIM_LWM_IRQ_HANDLER()
// {
//   HAL_TIM_IRQHandler( &tim_lwm_h );
// }


void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{

}


void HAL_GPIO_EXTI_Callback( uint16_t pin_bit )
{
  ledsx[0].set();
  bool need_stop { false };

  switch( pin_bit ) {
    case BTN_STOP_BIT:
      need_stop = true;
      break_flag = 7;
      break;

    default:
      ++UVAR_j;
      break;
  }

  if( need_stop ) {
    tim_lwm_stop();
  }
}


void EXTI0_IRQHandler()
{
  HAL_GPIO_EXTI_IRQHandler( BTN_STOP_BIT );
}

// ------------------------------------ ADC ------------------------------------------------

DMA_HandleTypeDef hdma_adc1;
ADC_HandleTypeDef hadc1;

void MX_DMA_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  HAL_NVIC_SetPriority( DMA2_Stream0_IRQn, 10, 0 );
  HAL_NVIC_EnableIRQ(   DMA2_Stream0_IRQn );
}

void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler( &hdma_adc1 );
}

int MX_ADC1_Init(void)
{
  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = ENABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 3;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection          = ADC_EOC_SEQ_CONV;
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK ) {
    errno = 5000; return 0;
  }

  ADC_ChannelConfTypeDef sConfig {
    .Channel      = ADC_CHANNEL_4,
    .Rank         = 1,
    .SamplingTime = ADC_SAMPLETIME_144CYCLES,
    .Offset       = 0
  };
  decltype( sConfig.Rank ) rank = 1;

  for( auto adc_ch : { ADC_CHANNEL_4, ADC_CHANNEL_5, ADC_CHANNEL_6 } ) {
    sConfig.Rank = rank;
    sConfig.Channel = adc_ch;
    if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK ) {
      errno = 5000 + rank; return 0;
    }
    ++rank;
  }

  return 1;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != ADC1 ) {
    return;
  }

  ADC_CLK_EN;
  ADC1_GPIO.cfgAnalog_N( ADC1_PINS );

  hdma_adc1.Instance                 = DMA2_Stream0;
  hdma_adc1.Init.Channel             = DMA_CHANNEL_0;
  hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_adc1.Init.Mode                = DMA_NORMAL;
  hdma_adc1.Init.Priority            = DMA_PRIORITY_MEDIUM;
  hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  if( HAL_DMA_Init( &hdma_adc1 ) != HAL_OK ) {
    errno = 5010; return;
  }

  __HAL_LINKDMA( adcHandle, DMA_Handle, hdma_adc1 );
  UVAR_j |= 2;

}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == ADC1 ) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    ADC1_GPIO.cfgIn_N( ADC1_PINS );
    HAL_DMA_DeInit( adcHandle->DMA_Handle );
  }
}

void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef* hadc1 )
{
  adc_dma_end = 1;
}


// ------------------------------------ Sensors classes ---------------------------------

int SensorAdc::init()
{
  return 1;
}

int SensorAdc::measure( int nx )
{
  if( nx < 1 ) {
    nx = 1;
  }
  ranges::fill( adc_data, 0 );

  for( int i=0; i<nx; ++i ) {
    adc_dma_end = 0;
    if( HAL_ADC_Start_DMA( &hadc1, (uint32_t*)adc_buf, ADC1_NCH ) != HAL_OK ) {
      errno = 4567;
      return 0;
    }
    for( int i=0; i<100000; ++i ) {
      if( adc_dma_end ) {
        break;
      }
    }
    if( ! adc_dma_end ) {
      errno = 4568;
      return 0;
    }
    for( unsigned j=0; j<std::size(adc_data); ++j ) {
      adc_data[j] += adc_buf[j];
    }
  }

  for( auto &x : adc_data ) {
    x /= nx;
  }

  return 1;
}

// ----------------------------------- 


int SensorAS5600::init()
{
  ang_sens.setCfg( AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off );
  return 1;
}

int SensorAS5600::measure( int /*nx*/ )
{
  iv = dev.getAngle() - zero_val;
  if( iv < -1024 ) {
    iv += 4096;
  }
  v  = (float)iv * k_a - 90.0f;
  return 1;
}

// ------------------------------------- Movers ----------------------------------------------

int Mover::move( float q, uint32_t t_cur )
{
  auto rc = move_do( q, t_cur );
  t_old = t_cur; q_last = q;
  return rc;
}

int MoverServo::move_do( float q, uint32_t t_cur )
{
  const uint32_t t_on = std::clamp(  (uint32_t) ( q * k_a + k_b ), t_on_min, t_on_max );
  setCtrlVal( t_on );
  return 1;
}


int MoverServoCont::move_do( float q, uint32_t t_cur )
{
  const float fbv = fb ? *fb : 0;
  const float dq = q - fbv;

  if( fabsf( dq ) < 1.0f ) { // dead zone, TODO: param
    setCtrlVal( t_on_cen );
    dbg_val0 = t_on_cen;
    return 1;
  }
  int32_t t_on = t_on_cen + (int) ( dq * t_on_dlt * k_a * UVAR_k / 1000 ); // TODO: param
  t_on = std::clamp( t_on, (int32_t)t_on_min, (int32_t)t_on_max );
  dbg_val0 = t_on;
  setCtrlVal( t_on );
  return 1;
}

int MoverServoCont::pre_run( float q_e, unsigned tp, uint32_t nn )
{
  setCtrlVal( t_on_cen );
  dbg_val0 = t_on_cen;
  return MoverServoBase::pre_run( q_e, tp, nn );
}

int MoverServoCont::post_run()
{
  setCtrlVal( t_on_cen );
  dbg_val0 = t_on_cen;
  return MoverServoBase::post_run();
}

// ------------------------------------  ------------------------------------------------


// ------------------------------------  ------------------------------------------------

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

