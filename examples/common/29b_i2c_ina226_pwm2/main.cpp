#include <cstring>
#include <cstdlib>
#include <cmath>

#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <oxc_ina226.h>

#include <../examples/common/inc/pwm2_ctl.h>

using namespace std;
using namespace SMLRL;

using sreal = StatData::sreal;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to misc PWM control with INA226 I2C sensor" NL;

TIM_HandleTypeDef tim_h;
using tim_ccr_t = decltype( tim_h.Instance->CCR1 );
void tim_cfg();

PWMData pwmdat( tim_h );

void handle_keys();

struct NamedFloat {
  const char *name;
  float *p;
  float (*get)();
  bool  (*set)( float v );
  uint32_t flags = 0;
  enum { flg_ro = 1 };
};

class NamedFloats {
  public:
   NamedFloats( NamedFloat *a_flts ) : fl( a_flts ) {};
   const NamedFloat* find( const char *nm ) const;
   bool  set( const char *nm, float v ) const; // changed variable, not NamedFloats object
   float get( const char *nm, float def = 0.0f, bool *ok = nullptr ) const;
  private:
   NamedFloat *fl;
};

const NamedFloat* NamedFloats::find( const char *nm ) const
{
  for( const auto *f = fl; f->name != nullptr; ++f ) {
    if( strcmp( nm, f->name ) == 0 ) {
      return f;
    }
  }
  return nullptr;
}

bool  NamedFloats::set( const char *nm, float v ) const
{
  auto f = find( nm );
  if( !f ) {
    return false;
  }
  if( f->flags & NamedFloat::flg_ro ) {
    return false;
  }
  if( f->set != nullptr ) {
    return f->set( v );
  }
  if( f->p != nullptr ) {
    *(f->p) = v;
    return true;
  }
  return false;
}

float NamedFloats::get( const char *nm, float def, bool *ok ) const
{
  bool l_ok;
  if( !ok ) {
    ok = &l_ok;
  }
  auto f = find( nm );
  if( !f ) {
    *ok = false;
    return def;
  }

  *ok = true;
  if( f->get != nullptr ) {
    return f->get();
  }
  if( f->p != nullptr ) {
    return *(f->p);
  }
  *ok = false;
  return def;
}

// ------------- floats values and get/set funcs -------------------

float W_max = 30.0f;

bool set_pwm_min( float v ) {
  pwmdat.set_min( v );
  return true;
}

float get_pwm_min() {
  return pwmdat.get_min();
}

bool set_pwm_max( float v ) {
  pwmdat.set_max( v );
  return true;
}

float get_pwm_max() {
  return pwmdat.get_max();
}

bool set_pwm_def( float v ) {
  pwmdat.set_def( v );
  return true;
}

float get_pwm_def() {
  return pwmdat.get_def();
}

NamedFloat flts[] = {
  {   "W_max",     &W_max,   nullptr,          nullptr, 0  },
  { "pwm_min",    nullptr,   get_pwm_min,  set_pwm_min, 0  },
  { "pwm_max",    nullptr,   get_pwm_max,  set_pwm_max, 0  },
  { "pwm_def",    nullptr,   get_pwm_def,  set_pwm_def, 0  },
  { nullptr, nullptr, nullptr, nullptr, 0  }
};

NamedFloats fl( flts );

// ---------------------------------------------


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] [skip_pwm] - measure V,I + control PWM"  };
int cmd_setcalibr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETCALIBR { "set_calibr", 'K', cmd_setcalibr, " I_lsb R_sh - calibrate for given shunt"  };
int cmd_tinit( int argc, const char * const * argv );
CmdInfo CMDINFO_TINIT { "tinit", 'I', cmd_tinit, " - reinit timer"  };
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", 0, cmd_pwm, " [val] - set PWM value"  };
int cmd_set_coeffs( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_COEFFS { "set_coeffs", 'F', cmd_set_coeffs, " k0 k1 k2 k3 - set ADC coeffs"  };
int cmd_set_float( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_FLOAT { "sf", 0, cmd_set_float, " name val - set float value"  };
int cmd_print_float( int argc, const char * const * argv );
CmdInfo CMDINFO_PRINT_FLOAT { "pf", 0, cmd_print_float, " [name] - print float values"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SETCALIBR,
  &CMDINFO_PWM,
  CMDINFOS_PWM,
  &CMDINFO_SET_COEFFS,
  &CMDINFO_SET_FLOAT,
  &CMDINFO_PRINT_FLOAT,
  nullptr
};

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
INA226 ina226( i2cd );
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
float v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };

bool isGoodINA226( INA226 &ina, bool print = true );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10; // 10 ms
  UVAR('n') = 1000000; // number of series (10ms 't' each): limited by steps
  UVAR('c') = 2; // n_ADC_ch_max;

  UVAR('p') = 0;     // PSC,  - max output freq
  UVAR('a') = 1439;  // ARR, to get 100 kHz with PSC = 0

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
  STDOUT_os;
  uint16_t id_manuf = ina.readReg( INA226::reg_id_manuf );
  uint16_t id_dev   = ina.readReg( INA226::reg_id_dev );
  if( print ) {
    os << "# id_manuf= " << HexInt16( id_manuf ) << "  id_dev= " << HexInt16( id_dev ) << NL;
  }
  if( id_manuf != INA226::id_manuf || id_dev != INA226::id_dev ) {
    if( print ) {
      os << "# Error: bad ids!" << NL;
    }
    return false;
  }
  return true;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  STDOUT_os;
  uint32_t t_step = UVAR('t');
  unsigned n_ch = 2;
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  bool skip_pwm = arg2long_d( 2, argc, argv, 0, 1, 1 ); // dont touch PWM

  StatData sdat( n_ch );

  ina226.setCfg( INA226::cfg_rst );
  uint16_t x_cfg = ina226.getCfg();
  os <<  NL "# getVIP: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;

  if( ! isGoodINA226( ina226, true ) ) {
    return 3;
  }

  uint16_t cfg = INA226::cfg_default;
  UVAR('e') = ina226.setCfg( cfg );
  x_cfg = ina226.getCfg();
  ina226.calibrate();
  os << "# cfg= " << HexInt16( x_cfg ) <<  " I_lsb_mA= " << ina226.get_I_lsb_mA()
     << " R_sh_uOhm= " << ina226.get_R_sh_uOhm() << NL;
  os << "# skip_pwm= " << skip_pwm << NL << "# Coeffs: ";
  for( decltype(+n_ch) j=0; j<n_ch; ++j ) {
    os << ' ' << v_coeffs[j];
  }
  os << NL;

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

    handle_keys();

    if( ! pwmdat.tick() ) {
      break;
    }

    float tc = 0.001f * ( tcc - tm00 );
    sreal v[n_ch+3]; // +1 for PWM +2=R +3=W

    if( UVAR('l') ) {  leds.set( BIT2 ); }

    // _g measn 'get' or 'measured'
    float V_g = ina226.getVbus_uV() * 1e-6f * v_coeffs[0];
    float I_g = ina226.getI_uA()    * 1e-6f * v_coeffs[1];
    float R_g = V_g / I_g;
    float W_g = V_g * I_g;
    v[0] = V_g;   v[1] = I_g;
    v[2] = pwmdat.get_v_real();
    v[3] = R_g;  v[4] = W_g;
    UVAR('z') = ina226.get_last_Vsh();

    if( UVAR('l') ) {  leds.reset( BIT2 ); }

    sdat.add( v );

    if( do_out ) {
      os <<  FltFmt( tc, cvtff_auto, 10, 3 );
      for( auto vc : v ) {
        os  << ' '  <<  vc;
      }
      os << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  pwmdat.end_run();

  sdat.calc();
  os << sdat << NL;

  delay_ms( 10 );

  return rc;
}


int cmd_setcalibr( int argc, const char * const * argv )
{
  float calibr_I_lsb = arg2float_d( 1, argc, argv, ina226.get_I_lsb_mA()  * 1e-3f, 1e-20f, 1e10f );
  float calibr_R     = arg2float_d( 2, argc, argv, ina226.get_R_sh_uOhm() * 1e-6f, 1e-20f, 1e10f );
  float V_sh_max =  INA226::lsb_V_sh_nv * 1e-9f * 0x7FFF;
  STDOUT_os;
  ina226.set_calibr_val( (uint32_t)(calibr_R * 1e6f), (uint32_t)(calibr_I_lsb * 1e3f) );
  os << "# calibr_I_lsb= " << calibr_I_lsb << " calibr_R= " << calibr_R
     << " V_sh_max=  " << V_sh_max
     << " I_max= " << ( V_sh_max / calibr_R ) << " / " << ( calibr_I_lsb * 0x7FFF ) << NL;
  return 0;
}


// ------------------------------------------- PWM ---------------------------------------

void tim_cfg()
{
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = UVAR('p');
  tim_h.Init.Period            = UVAR('a');
  tim_h.Init.ClockDivision     = 0;
  tim_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim_h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_h, &sClockSourceConfig );

  int pbase = UVAR('a');
  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;


  HAL_TIM_PWM_Stop( &tim_h, TIM_CHANNEL_1 );
  tim_oc_cfg.Pulse = (tim_ccr_t)( pwmdat.get_def() * pbase / 100 );
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }
  HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 );

}

int cmd_pwm( int argc, const char * const * argv )
{
  float vmin = fl.get( "pwm_min", 3.0f );
  float vdef = fl.get( "pwm_def", 5.0f );
  float vmax = fl.get( "pwm_max", 70.0f );
  float v = arg2float_d( 1, argc, argv, vdef, vmin, vmax );
  STDOUT_os;
  pwmdat.set_v_manual( v );
  tim_print_cfg( TIM_EXA );
  os << NL "# PWM:  in: " << pwmdat.get_v() << "  real: " << pwmdat.get_v_real() << NL;
  return 0;
}



int cmd_tinit( int argc, const char * const * argv )
{
  tim_cfg();
  tim_print_cfg( TIM_EXA );

  return 0;
}

int cmd_set_coeffs( int argc, const char * const * argv )
{
  if( argc > 1 ) {
    v_coeffs[0] = arg2float_d( 1, argc, argv, 1, -1e10f, 1e10f );
    v_coeffs[1] = arg2float_d( 2, argc, argv, 1, -1e10f, 1e10f );
    v_coeffs[2] = arg2float_d( 3, argc, argv, 1, -1e10f, 1e10f );
    v_coeffs[3] = arg2float_d( 4, argc, argv, 1, -1e10f, 1e10f );
  }
  STDOUT_os;
  os << "# Coefficients: "
     << v_coeffs[0] << ' ' << v_coeffs[1] << ' ' << v_coeffs[2] << ' ' << v_coeffs[3] << NL;
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

// ------------------ floats -----------------------
int cmd_set_float( int argc, const char * const * argv )
{
  STDOUT_os;
  if( argc < 3 ) {
    os << "# error: 2 arguments requred" << NL;
    return 1;
  }
  float v = arg2float_d( 2, argc, argv, 0.0f, -FLT_MAX, FLT_MAX );
  bool rc = fl.set( argv[1], v );
  return rc ? 0 : 2;
}

int cmd_print_float( int argc, const char * const * argv )
{
  STDOUT_os;
  if( argc > 1 ) {
    const char *nm = argv[1];
    bool ok;
    float v = fl.get( nm, 0.0f, &ok );
    if( !ok ) {
      os << "# error: name \"" << nm << "\" not found" << NL;
      return 1;
    }
    os << nm << " = " << v << NL;
    return 0;
  }

  // all vars TODO: iface
  for( auto f : flts ) {
    if( ! f.name ) {
      break;
    }
    bool ok;
    float v = fl.get( f.name, 0.0f, &ok );
    if( ok ) {
      os << f.name << " = " << v << NL;
    }
  }
  return 0;
}

// ------------------ misc tests -----------------------



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

