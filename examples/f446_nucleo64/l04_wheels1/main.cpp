#include <stdlib.h>
#include <oxc_auto.h>
#include <oxc_tim.h>

#include "wheels_pins.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Test model car: motors, sensors...." NL;

RunState rs;


PinsOut motor_dir( GpioC, 5, 5 );
uint8_t calc_dir_bits( int r_w, int l_w ); // from enum motor_bits

PinsIn proxy_sens( GpioB, 12, 4 );
int is_proxy_obstacle();
int us_forward_min = 100; // minimal distance via US while forward moving TODO: adjust

TIM_HandleTypeDef tim1_h, tim3_h, tim4_h, tim14_h;
void tim1_cfg(); // PWM (1,2), US: (pulse: 3, echo: 4 )
void tim3_cfg(); // count( R )
void tim4_cfg();  // count( L )
void tim14_cfg(); // servo( 1 )
const int tim1_period = 8500; // approx 20Hz
void set_motor_pwm( int r, int l ); // 0-100 %, and update state

void set_us_dir( int dir ); // -90:90
int us_dir_zero = 1420; // CCR units
int us_dir_scale = 10;
const int us_scan_min = -80, us_scan_max = 80, us_scan_step = 10, // degree
          us_scan_n = 1 + ( (us_scan_max - us_scan_min) / us_scan_step ) ;
int us_scans[ us_scan_n ];

volatile int us_dir {0}, us_l {0}, us_l0 {0}, us_i {0};
int read_new_us_l();

// run steps

const int max_run_steps { 20 };
int n_run_steps { 0 };
RunStepData run_steps[max_run_steps];

int run_single_step( int n );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_go( int argc, const char * const * argv );
CmdInfo CMDINFO_GO { "go", 'g', cmd_go, " time [right=50] [left=right]"  };
int cmd_us_dir( int argc, const char * const * argv );
CmdInfo CMDINFO_US_DIR { "us_dir", 0, cmd_us_dir, " [dir=0] (-90:90)"  };
int cmd_us_scan( int argc, const char * const * argv );
CmdInfo CMDINFO_US_SCAN { "us_scan", 'U', cmd_us_scan, " - scan via US sensor"  };
int cmd_set_step( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_STEP { "set_step", 'P', cmd_set_step, " n l_r l_l p_c t_max p_c0 t_c0 - set run step"  };
int cmd_print_steps( int argc, const char * const * argv );
CmdInfo CMDINFO_PRINT_STEPS { "print_steps", 'O', cmd_print_steps, " - print steps data"  };
int cmd_run_steps( int argc, const char * const * argv );
CmdInfo CMDINFO_RUN_STEPS { "run_steps", 'R', cmd_run_steps, " [n_s] [n_e] - run steps"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_GO,
  &CMDINFO_US_DIR,
  &CMDINFO_US_SCAN,
  &CMDINFO_SET_STEP,
  &CMDINFO_PRINT_STEPS,
  &CMDINFO_RUN_STEPS,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 ); // zero add means no real device


int main(void)
{
  BOARD_PROLOG;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  UVAR('t') = 1000; // default 'go' time
  UVAR('g') =  100; // 'go' tick
  UVAR('w') =   40; // default 'go' PWM
  UVAR('n') =   10; // unused
  UVAR('o') =    1; // ignore proxymity sensors

  motor_dir.initHW();
  motor_dir.reset( 0x1F );
  proxy_sens.initHW();

  // UVAR('e') = i2c_default_init( i2ch );
  // i2c_dbg = &i2cd;
  // i2c_client_def = &XXXXX;

  tim1_cfg();
  tim3_cfg();
  tim4_cfg();
  tim14_cfg();

  rs.reset();

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );
  return 0;
}


void print_tim_info( TIM_TypeDef *tim, const char *tname )
{
  std_out << "# " << tname;
  tim_print_cfg( tim );
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  std_out <<  NL "Test0: " NL;
  print_tim_info( TIM1, "TIM1=(PWM,US)" );
  print_tim_info( TIM_N_R, "TIM3=TIM_N_R" );
  print_tim_info( TIM_N_L, "TIM4=TIM_B_L" );
  print_tim_info( TIM_SERVO, "TIM14=TIM_SERVO" );
  return 0;
}

void set_motor_pwm( int r, int l )
{
  const auto peri = TIM1->ARR;
  TIM1->CCR1 = peri * abs(r) / 100;
  TIM1->CCR2 = peri * abs(l) / 100;
  TIM1->CNT  = 0;
  rs.r_w = r; rs.l_w = l;
}

uint8_t calc_dir_bits( int r, int l )
{
  uint8_t bits {0};
  if( r > 0 ) {
    bits = motor_bit_r;
  } else if ( r < 0 ) {
    bits = motor_bit_rn;
  } // if 0 - keep zero

  if( l > 0 ) {
    bits |= motor_bit_l;
  } else if ( l < 0 ) {
    bits |= motor_bit_ln;
  }

  return bits;
}

int read_new_us_l()
{
  int us_i_0 { us_i };
  for( int i = 0; i < 1000; ++i ) {
    if( us_i != us_i_0 ) {
      return us_l;
    }
    delay_ms( 1 );
  }
  return us_l; // fallback
}

// returns 0 if clear or all bits if obstavle for given move
int is_proxy_obstacle()
{
  uint16_t prox = (~proxy_sens.read()) & PROXY_ALL; // inverse sensors
  rs.prox = prox;
  if( UVAR('o') ) {
    return 0;
  }
  if( prox == 0 ) { // optimization: all clear
    return 0;
  }

  if( ( rs.r_w > 0 && ( prox & PROXY_FR ) ) ||
      ( rs.r_w < 0 && ( prox & PROXY_BR ) ) ||
      ( rs.l_w > 0 && ( prox & PROXY_FL ) ) ||
      ( rs.l_w < 0 && ( prox & PROXY_BL ) )    ) {
    leds.set( 8 );
    std_out <<  "Prox: "  << HexInt( prox )  <<  NL;
    return prox;
  }
  return 0;
}

// --------------------------- commands ---------------------------------------

int cmd_go( int argc, const char * const * argv )
{
  int t    = arg2long_d( 1, argc, argv, UVAR('t'),  -1, 100000 );
  int r_w  = arg2long_d( 2, argc, argv, UVAR('w'), -100,  100 );
  int l_w  = arg2long_d( 3, argc, argv,       r_w, -100,  100 );
  int go_tick = UVAR('g');
  if( t < 0 ) { // -1 = default time flag
    t = UVAR('t');
  }

  std_out <<  NL "go: t= "  <<  t  <<  " r= "  <<  r_w  <<  " l= "  <<  l_w  << NL;

  if( us_dir != 0 ) {
    set_us_dir( 0 );
    delay_ms( 500 );
  }

  leds.reset( 9 );
  motor_dir.write( calc_dir_bits( r_w, l_w ) );
  set_motor_pwm( r_w, l_w );

  bool proxy_flag = false;
  TIM_N_L->CNT = 0; TIM_N_R->CNT = 0; // reset wheel tick counters

  for( ; t > 0 && !break_flag && !proxy_flag; t -= go_tick ) {

    if( ( r_w + l_w ) > 5 && us_l0 < us_forward_min ) {
      leds.set( 1 );
      std_out <<  "Minimal forward US distance detected "  << us_l0 <<  NL;
      break;
    }

    if( is_proxy_obstacle() ) {
      proxy_flag = true;
      continue; // really break;
    }

    delay_ms( t > go_tick ? go_tick : t );
  }

  motor_dir.reset( motor_bits );
  set_motor_pwm( 0, 0 );
  if( break_flag ) {
    std_out <<  "Break!" NL;
  }

  uint16_t cnt_l = TIM_N_L->CNT;
  uint16_t cnt_r = TIM_N_R->CNT;
  std_out <<  "Counts: right: "  <<  cnt_r << " left: " <<  cnt_l  <<  NL;

  return 0;
}

void set_us_dir( int dir )
{
  if( dir < -100 ) { dir = -100; }
  if( dir >  100 ) { dir =  100; }
  uint32_t d = us_dir_zero + dir * us_dir_scale; // TODO: calibrate

  TIM_SERVO->CCR1 = d;
  TIM_SERVO->CNT  = 0;
  delay_ms( 10 );

  UVAR('d') = d;
  us_dir = dir;
}


int cmd_us_dir( int argc, const char * const * argv )
{
  int dir  = arg2long_d( 1, argc, argv,  0,  -100, 100 );

  set_us_dir( dir );

  std_out <<  NL "us_dir: "  <<  us_dir << NL;

  return 0;
}

int cmd_us_scan( int argc, const char * const * argv )
{
  std_out <<  NL "us_scan: "  <<  us_dir << NL;
  set_us_dir( us_scan_min ); // to settle before
  delay_ms( 300 );
  for( int i=0, d = us_scan_min; i < us_scan_n && d <= us_scan_max; ++i, d += us_scan_step ) {
    set_us_dir( d );
    delay_ms( 200 );
    int l = read_new_us_l();
    us_scans[ i ] = l;
    std_out <<  d  <<  ' '  <<  l  <<  NL;
  }
  set_us_dir( 0 );
  delay_ms( 500 );

  return 0;
}

int cmd_set_step( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv,  -1, -1,  max_run_steps-1 );
  if( n < 0 ) {
    n = n_run_steps;
  }
  if( n >= max_run_steps ) {
    std_out << "# error: n too large: " << n << NL;
    return 1;
  }

  RunStepData &s = run_steps[n];

  s.l_r = arg2long_d( 2, argc, argv,     50, -20000,  20000 );
  s.l_l = arg2long_d( 3, argc, argv,  s.l_r, -20000,  20000 );
  s.p_c = arg2long_d( 4, argc, argv, 30,  0,  100 );
  s.t_max = arg2long_d( 5, argc, argv, 10000,  0,  100000 );
  s.p_c0 = arg2long_d( 6, argc, argv, -1,  0,  100 );
  s.t_0 = arg2long_d( 7, argc, argv, -1,  0,  s.t_max );

  if( n_run_steps <= n ) {
    n_run_steps = n+1;
  }
  std_out << "# " << n << ' ' << s << NL;
  return 0;
}

int cmd_print_steps( int argc, const char * const * argv )
{
  for( int i=0; i< n_run_steps; ++i ) {
    std_out << "# " << i << ' ' <<  run_steps[i] << NL;
  }
  return 0;
}

int cmd_run_steps( int argc, const char * const * argv )
{
  int n_s = arg2long_d( 1, argc, argv,  0,               0,   n_run_steps-1 );
  int n_e = arg2long_d( 2, argc, argv,  n_s, n_run_steps-1, n_run_steps-1 );
  std_out << "# run steps: " << n_s << " ... " << n_e << NL;

  for( int i=n_s; i<n_e; ++i ) {
    auto rc = run_single_step( i );
    std_out << "# i= " << i << " rc= " << rc << NL;
    if( rc != 0 ) {
      break;
    }
  }
  // stop motors
  return 0;
}


void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )
{
  static uint32_t c_old = 0xFFFFFFFF;
  if( htim->Channel != HAL_TIM_ACTIVE_CHANNEL_4 )  { // only US sensor
    return;
  }

  leds.toggle( BIT1 );
  uint32_t cap2 = HAL_TIM_ReadCapturedValue( htim, TIM_CHANNEL_4 );
  if( cap2 > c_old ) {
    uint32_t l = cap2 - c_old;
    us_l = UVAR('c') = l;
    if( us_dir == 0 ) {
      us_l0 = UVAR('l') = l;
    }
    ++us_i;
    // leds.toggle( BIT2 );
  }
  c_old = cap2;
}


// ----------------------------------- steps
void RunStepData::print( OutStream &os ) const
{
  os << l_r << ' ' << l_l << ' ' << p_c << ' ' << t_max << ' '
          << p_c0 << ' ' << t_0;
}

int run_single_step( int n )
{
  if( n >= n_run_steps || n < 0 ) {
    return 1;
  }
  const RunStepData &sd = run_steps[n];
  std_out << "# " << n << ' ' << sd << NL;

  // TODO: check previous condition if exists and seed (need state)

  int tick_r = abs(sd.l_r) * tick_per_turn / wheel_len;
  int tick_l = abs(sd.l_l) * tick_per_turn / wheel_len;
  std_out << "# ticks: r: " << tick_r << " l: " << tick_l << NL;
  // calc start and run params
  // run
  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc


