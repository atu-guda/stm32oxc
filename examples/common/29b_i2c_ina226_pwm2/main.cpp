#include <cstring>
#include <cstdlib>
#include <cmath>

#include <algorithm>
#include <iterator>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>

#include <oxc_ina226.h>

#include <../examples/common/inc/pwm2_ctl.h>

#define debug (UVAR('d'))

#define WAIT_BIT BIT2

using namespace std;
using namespace SMLRL;

using sreal = StatData::sreal;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to misc PWM control with INA226 I2C sensor" NL;

TIM_HandleTypeDef tim_h;
using tim_ccr_t = decltype( TIM_EXA->CCR1 );
void tim_cfg();
void do_set_pwm( float v );
bool measure_and_calc( float *v );

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
INA226 ina226( i2cd );
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c'). not ADC: INA226
float v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };

bool isGoodINA226( INA226 &ina, bool print = true );

PWMInfo pwminfo( 0.09347f, -0.671f, 0.1139f );

PWMData pwmdat( pwminfo, do_set_pwm );

void handle_keys();

bool print_var( const char *nm );
bool set_var( const char *nm, const char *s );


// ------------- floats values and get/set funcs -------------------

bool set_pwm_min( float v, int /* idx */ )
{
  pwmdat.set_pwm_min( v );
  return true;
}

float get_pwm_min( int /* idx */ )
{
  return pwmdat.get_pwm_min();
}

bool set_pwm_max( float v, int /* idx */  )
{
  pwmdat.set_pwm_max( v );
  return true;
}

float get_pwm_max( int /* idx */ )
{
  return pwmdat.get_pwm_max();
}

bool set_pwm_def( float v, int /* idx */  )
{
  pwmdat.set_pwm_def( v );
  return true;
}

float get_pwm_def( int /* idx */ )
{
  return pwmdat.get_pwm_def();
}

constexpr NamedFloat ob_v_coeffs   {   "v_coeffs",            v_coeffs, size(v_coeffs)  };
constexpr NamedFloat ob_W_max      {      "W_max",      &pwminfo.W_max  };
constexpr NamedFloat ob_V_max      {      "V_max",      &pwminfo.V_max  };
constexpr NamedFloat ob_I_max      {      "I_max",      &pwminfo.I_max  };
constexpr NamedFloat ob_R_max      {      "R_max",      &pwminfo.R_max  };
constexpr NamedFloat ob_V_00       {       "V_00",       &pwminfo.V_00  };
constexpr NamedFloat ob_R_0        {        "R_0",        &pwminfo.R_0  };
constexpr NamedFloat ob_k_gv1      {      "k_gv1",      &pwminfo.k_gv1  };
constexpr NamedFloat ob_k_gv2      {      "k_gv2",      &pwminfo.k_gv2  };
constexpr NamedFloat ob_x_0        {        "x_0",        &pwminfo.x_0  };
constexpr NamedFloat ob_ki_v       {       "ki_v",       &pwminfo.ki_v  };
constexpr NamedFloat ob_k_move     {     "k_move",       &pwminfo.k_move  };
constexpr NamedFloat ob_cal_min    {    "cal_min",       &pwminfo.cal_min  };
constexpr NamedFloat ob_cal_step   {   "cal_step",       &pwminfo.cal_step };
constexpr NamedFloat ob_cal_pwm    {    "cal_pwm",       pwminfo.d_pwm, size(pwminfo.d_pwm) };
constexpr NamedFloat ob_cal_v      {      "cal_v",         pwminfo.d_v, size(pwminfo.d_v)   };
constexpr NamedFloat ob_rehint_lim { "rehint_lim", &pwminfo.rehint_lim  };
constexpr NamedFloat ob_regre_lev  {  "regre_lev", &pwminfo.regre_lev  };
constexpr NamedFloat ob_pwm_min    {    "pwm_min",         get_pwm_min,  set_pwm_min  };
constexpr NamedFloat ob_pwm_max    {    "pwm_max",         get_pwm_max,  set_pwm_max  };
constexpr NamedFloat ob_pwm_def    {    "pwm_def",         get_pwm_def,  set_pwm_def  };

constexpr const NamedObj *const objs_info[] = {
  & ob_v_coeffs,
  & ob_W_max,
  & ob_V_max,
  & ob_I_max,
  & ob_R_max,
  & ob_V_00,
  & ob_R_0,
  & ob_k_gv1,
  & ob_k_gv2,
  & ob_x_0,
  & ob_ki_v,
  & ob_k_move,
  & ob_cal_min,
  & ob_cal_step,
  & ob_cal_pwm,
  & ob_cal_v,
  & ob_rehint_lim,
  & ob_regre_lev,
  & ob_pwm_min,
  & ob_pwm_max,
  & ob_pwm_def,
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



// ---------------------------------------------


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] [skip_pwm] - measure V,I + control PWM"  };
int cmd_setcalibr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETCALIBR { "set_calibr", 'K', cmd_setcalibr, " I_lsb R_sh - calibrate INA226 for given shunt"  };
int cmd_tinit( int argc, const char * const * argv );
CmdInfo CMDINFO_TINIT { "tinit", 'I', cmd_tinit, " - reinit timer"  };
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", 0, cmd_pwm, " [val] - set PWM value"  };
int cmd_calibrate( int argc, const char * const * argv );
CmdInfo CMDINFO_CALIBRATE { "calibrate", 'C', cmd_calibrate, " [pwm_max] [dt] [fake] - calibrate PWM values"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  // DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TINIT,
  &CMDINFO_SETCALIBR,
  &CMDINFO_PWM,
  &CMDINFO_CALIBRATE,
  CMDINFOS_PWM,
  nullptr
};

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10; // 10 ms
  UVAR('n') = 1000000; // number of series (10ms 't' each): limited by steps

  UVAR('p') = 0;     // PSC,  - max output freq
  UVAR('f') = 100000;// PWM freq: to calculate ARR

  pwminfo.R_max = 200.0f;
  pwminfo.V_max =   8.0f;
  pwminfo.I_max =  50.0f;
  pwminfo.W_max = 200.0f;

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  tim_cfg();

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ina226;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}

bool isGoodINA226( INA226 &ina, bool print )
{
  uint16_t id_manuf = ina.readReg( INA226::reg_id_manuf );
  uint16_t id_dev   = ina.readReg( INA226::reg_id_dev );
  if( print ) {
    std_out << "# id_manuf= " << HexInt16( id_manuf ) << "  id_dev= " << HexInt16( id_dev ) << NL;
  }
  if( id_manuf != INA226::id_manuf || id_dev != INA226::id_dev ) {
    if( print ) {
      std_out << "# Error: bad V/I sensor ids!" << NL;
    }
    return false;
  }
  if( debug > 0 ) {
    std_out << "# INA226 detected" NL;
  }
  return true;
}

bool init_INA()
{
  ina226.setCfg( INA226::cfg_rst );
  if( ! isGoodINA226( ina226, true ) ) {
    return false;
  }
  uint16_t x_cfg = ina226.getCfg();
  std_out <<  "# init_INA:  cfg= " <<  HexInt16( x_cfg ) << NL;


  uint16_t cfg = INA226::cfg_default;
  UVAR('e') = ina226.setCfg( cfg );
  x_cfg = ina226.getCfg();
  ina226.calibrate();
  std_out << "# cfg= " << HexInt16( x_cfg ) <<  " I_lsb_mA= " << ina226.get_I_lsb_mA()
     << " R_sh_uOhm= " << ina226.get_R_sh_uOhm() << NL;
  return true;
}

bool measure_and_calc( float *v )
{
  if( !v ) {
    return false;
  }

  float V_g = ina226.getVbus_uV() * 1e-6f * v_coeffs[0];
  float I_g = ina226.getI_uA()    * 1e-6f * v_coeffs[1];
  float R_g = ( I_g > 1e-6f ) ? ( V_g / I_g ) : 1.0e9f;
  float W_g = V_g * I_g;

  v[didx_v]   = V_g;
  v[didx_i]   = I_g;
  v[didx_pwm] = pwmdat.get_pwm_real();
  v[didx_r]   = R_g;
  v[didx_w]   = W_g;
  v[didx_val] = pwmdat.get_v();

  UVAR('z') = ina226.get_last_Vsh();
  return true;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR('t');
  unsigned n_ch = 2;
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  bool skip_pwm = arg2long_d( 2, argc, argv, 0, 1, 1 ); // dont touch PWM

  StatData sdat( n_ch );

  if( ! init_INA() ) {
    return 3;
  }

  std_out <<  "# test0: n= " <<  n <<  " t= " <<  t_step <<  " skip_pwm= " << skip_pwm << NL << "# Coeffs: ";
  for( decltype(+n_ch) j=0; j<n_ch; ++j ) {
    std_out << ' ' << v_coeffs[j];
  }
  std_out << NL "#        t          V          I        pwm          R          W        val"  NL;

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  pwmdat.prep( t_step, skip_pwm );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR('b');

  break_flag = 0;
  for( decltype(+n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }
    float tc = 0.001f * ( tcc - tm00 );

    float v[didx_n];
    measure_and_calc( v );

    sdat.add( v );

    if( do_out ) {
      std_out <<  FltFmt( tc, cvtff_auto, 10, 3 );
      for( auto vc : v ) {
        std_out  << ' '  <<  FltFmt( vc, cvtff_auto, 10 );
      }
      std_out << NL;
    }

    handle_keys();

    if( ! pwmdat.tick( v ) ) {
      std_out << "# tick break: reason:" << pwmdat.get_reason() << NL;
      break;
    }

    if( UVAR('l') ) {  leds.set( WAIT_BIT ); }
    delay_ms_until_brk( &tm0, t_step );
    if( UVAR('l') ) {  leds.reset( WAIT_BIT ); }
  }

  pwmdat.end_run();

  sdat.calc();
  std_out << sdat << NL;

  delay_ms( 10 );

  return rc;
}


int cmd_setcalibr( int argc, const char * const * argv )
{
  float calibr_I_lsb = arg2float_d( 1, argc, argv, ina226.get_I_lsb_mA()  * 1e-3f, 1e-20f, 1e10f );
  float calibr_R     = arg2float_d( 2, argc, argv, ina226.get_R_sh_uOhm() * 1e-6f, 1e-20f, 1e10f );
  float V_sh_max =  INA226::lsb_V_sh_nv * 1e-9f * 0x7FFF;
  ina226.set_calibr_val( (uint32_t)(calibr_R * 1e6f), (uint32_t)(calibr_I_lsb * 1e3f) );
  std_out << "# calibr_I_lsb= " << calibr_I_lsb << " calibr_R= " << calibr_R
     << " V_sh_max=  " << V_sh_max
     << " I_max= " << ( V_sh_max / calibr_R ) << " / " << ( calibr_I_lsb * 0x7FFF ) << NL;
  return 0;
}


// ------------------------------------------- PWM ---------------------------------------

void tim_cfg()
{
  // not use all functions from oxc_tim: time may be not initialized here
  uint32_t in_freq = get_TIM_in_freq( TIM_EXA );
  uint32_t psc = UVAR('p');
  uint32_t cnt_freq = in_freq / ( psc + 1 );
  uint32_t arr = cnt_freq / UVAR('f') - 1;

  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = psc;
  tim_h.Init.Period            = arr;
  tim_h.Init.ClockDivision     = 0;
  tim_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim_h ) != HAL_OK ) {
    UVAR('e') = 111; // like error
    return;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_h, &sClockSourceConfig );

  HAL_TIM_PWM_Stop( &tim_h, TIM_CHANNEL_1 );

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  tim_oc_cfg.Pulse = (tim_ccr_t)( pwmdat.get_pwm_min() * arr / 100 );

  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 112;
    return;
  }
  HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 );

}

void do_set_pwm( float v )
{
  uint32_t scl = TIM_EXA->ARR;
  auto nv = (tim_ccr_t)( v * scl / 100 );
  if( nv != TIM_EXA->CCR1 ) {
    TIM_EXA->CCR1 = nv;
  }
}

int cmd_pwm( int argc, const char * const * argv )
{
  float vmin  = pwmdat.get_pwm_min();
  float vdef  = pwmdat.get_pwm_def();
  float vmax  = pwmdat.get_pwm_max();

  float v = arg2float_d( 1, argc, argv, vdef, vmin, vmax );
  pwmdat.set_pwm_manual( v );
  tim_print_cfg( TIM_EXA );
  std_out << NL "# PWM:  in: " << pwmdat.get_v() << "  real: " << pwmdat.get_pwm_real() << NL;
  return 0;
}

// TODO: to PWMData
int cmd_calibrate( int argc, const char * const * argv )
{
  float vmin = pwminfo.cal_min;
  float vmax_def = pwmdat.get_pwm_max();
  float vmax = arg2float_d( 1, argc, argv, 0.6f * vmax_def, 2.0f, vmax_def );
  unsigned dt  = arg2long_d( 2, argc, argv, 10000, 1000, 200000 );
  unsigned fake_cal = arg2long_d( 3, argc, argv, 0, 0, 1 );

  const unsigned n_measure = 10; // TODO: params

  if( !fake_cal && ! init_INA() ) {
    return 3;
  }

  pwminfo.clearCalibrationArr();

  unsigned n_steps = (unsigned) ceilf( ( vmax - vmin ) / pwminfo.cal_step );
  if( n_steps >= PWMInfo::max_cal_steps ) {
    n_steps = PWMInfo::max_cal_steps;
  }

  float R_0_c = pwminfo.R_0;

  std_out << "# Calibrating: vmin=" << vmin << " vmax= " << vmax << " n_steps= " << n_steps
          << NL "# N         pwm           V           I           R           W" << NL;

  break_flag = 0;
  for( unsigned i=0; i<n_steps && !break_flag; ++i ) {
    float v[didx_n];
    float pwm_v = vmin + i * pwminfo.cal_step;
    float c_pwm, c_v_a = 0, c_i_a = 0; // real / average values

    if( !fake_cal ) {
      pwmdat.set_pwm_manual( pwm_v );

      for( unsigned j=0; j<dt && !break_flag; j+=10 ) { // wait for steady + check all limits
        if( delay_ms_brk( 10 ) != 0 ) {
          break;
        }
        measure_and_calc( v );
        auto rc = pwmdat.check_lim( v );
        if( rc != PWMData::check_result::ok ) {
          break_flag = 2;
          std_out << "# Error: limits! " << NL;
          break;
        }
      }
      if( break_flag ) { break; }

      c_pwm = pwmdat.get_pwm_real();
      float c_v = 0, c_i = 0;

      for( unsigned j=0; j<n_measure && !break_flag; ++j ) {
        if( delay_ms_brk( 50 ) != 0 ) {
          break;
        }
        measure_and_calc( v );
        auto rc = pwmdat.check_lim( v );
        if( rc != PWMData::check_result::ok ) {
          break_flag = 2;
          std_out << "# Error: limits! " << NL;
          break;
        }
        c_v += v[didx_v];
        c_i += v[didx_i];
      }
      c_v_a = c_v / n_measure;
      c_i_a = c_i / n_measure;
    } else { // fake_cal: values must not changed
      c_pwm     = pwm_v;
      c_v_a     = pwminfo.pwm2V( c_pwm );
      c_i_a     = c_v_a / pwminfo.R_0;
      v[didx_r] = pwminfo.R_0;
      v[didx_w] = c_v_a * c_i_a;

    }

    if( i == 0 ) {
      R_0_c = c_v_a / c_i_a;
    }

    pwminfo.addCalibrationStep( c_pwm, c_v_a, c_i_a );

    std_out << "# " << i << ' ' << c_pwm << ' ' << c_v_a << ' ' << c_i_a << ' '
       << v[didx_r] << ' ' << v[didx_w] << NL;
  }

  pwmdat.set_pwm_manual( pwmdat.get_pwm_min() );

  if( break_flag ) {
    std_out << "# calibrtion exit by break!" << NL;
    return 2;
  }

  float err_max = 0;

  bool ok = pwminfo.calcCalibration( err_max, R_0_c, fake_cal );

  return ok ? 0 : 5;
}



int cmd_tinit( int argc, const char * const * argv )
{
  tim_cfg();
  tim_print_cfg( TIM_EXA );

  return 0;
}

void handle_keys()
{
  auto v = tryGet( 0 );
  if( !v.good() ) {
    return;
  }

  switch( v.c ) {
    case 'w': pwmdat.add_to_hand(  1 );  break;
    case 'W': pwmdat.add_to_hand(  5 );  break;
    case 's': pwmdat.add_to_hand( -1 );  break;
    case 'S': pwmdat.add_to_hand( -5 );  break;
    case 'z': pwmdat.set_hand( 0 );      break;
    case '0': pwmdat.adj_hand_to(  0 );  break;
    case '1': pwmdat.adj_hand_to( 10 );  break;
    case '2': pwmdat.adj_hand_to( 20 );  break;
    case '3': pwmdat.adj_hand_to( 30 );  break;
    case '4': pwmdat.adj_hand_to( 40 );  break;
    case '5': pwmdat.adj_hand_to( 50 );  break;
    case 'g': pwmdat.set_t_mul( 0 ); break;
    case 'G': pwmdat.set_t_mul( 1 ); break;
    case 'f': pwmdat.set_t_mul( 2 ); break;
    case 'F': pwmdat.set_t_mul( 5 ); break;
    default: break;
  }

}


// ------------------ misc tests -----------------------



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

