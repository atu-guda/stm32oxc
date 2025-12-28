#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <oxc_ina226.h>

#include <../examples/common/inc/pwm1_ctl.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test INA226 I2C device with  PWM control" NL;

TIM_HandleTypeDef tim_h;
using tim_ccr_t = decltype( tim_h.Instance->CCR1 );
void tim_cfg();


PWMData pwmdat( tim_h );

void handle_keys();



// --- local commands;
DCL_CMD_REG( test0, 'T', " [n] [skip_pwm] - measure V,I + control PWM"  );
DCL_CMD_REG( set_calibr, 'K', " I_lsb R_sh - calibrate for given shunt"  );
DCL_CMD_REG( tinit, 'I', " - reinit timer"  );
DCL_CMD_REG( pwm, 'W', " [val] - set PWM value"  );
DCL_CMD_REG( set_coeffs, 'F', " k0 k1 k2 k3 - set ADC coeffs"  );


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
INA226 ina226( i2cd );
const uint32_t n_ADC_ch_max = 4; // current - in UVAR_c
xfloat v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };

bool isGoodINA226( INA226 &ina, bool print = true );


int main(void)
{
  BOARD_PROLOG;

  UVAR_t =      10; // 10 ms
  UVAR_n = 1000000; // number of series (10ms 't' each): limited by steps
  UVAR_c =       2; // n_ADC_ch_max;

  UVAR_p = 0;     // PSC,  - max output freq
  UVAR_a = 1439;  // ARR, to get 100 kHz with PSC = 0

  tim_cfg();

  UVAR_e = i2c_default_init( i2ch /*, 400000 */ );
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
      std_out << "# Error: bad ids!" << NL;
    }
    return false;
  }
  return true;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR_t;
  unsigned n_ch = 2;
  uint32_t n = arg2long_d( 1, argc, argv, UVAR_n, 1, 1000000 ); // number of series

  bool skip_pwm = arg2long_d( 2, argc, argv, 0, 1, 1 ); // dont touch PWM

  StatData sdat( n_ch );

  ina226.setCfg( INA226::cfg_rst );
  uint16_t x_cfg = ina226.getCfg();
  std_out <<  NL "# getVIP: n= " <<  n <<  " t= " <<  t_step <<  "  cfg= " <<  HexInt16( x_cfg ) << NL;

  if( ! isGoodINA226( ina226, true ) ) {
    return 3;
  }

  uint16_t cfg = INA226::cfg_default;
  UVAR_e = ina226.setCfg( cfg );
  x_cfg = ina226.getCfg();
  ina226.calibrate();
  std_out << "# cfg= " << HexInt16( x_cfg ) <<  " I_lsb_mA= " << ina226.get_I_lsb_mA()
     << " R_sh_uOhm= " << ina226.get_R_sh_uOhm() << NL;
  std_out << "# skip_pwm= " << skip_pwm << NL << "# Coeffs: ";
  for( decltype(n_ch) j=0; j<n_ch; ++j ) {
    std_out << ' ' << v_coeffs[j];
  }
  std_out << NL;

  leds.set(   0x07_mask ); delay_ms( 100 );  leds.reset( 0x07_mask  );

  pwmdat.prep( t_step, skip_pwm );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR_b;

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    handle_keys();

    if( ! pwmdat.tick() ) {
      break;
    }

    xfloat tc = 0.001f * ( tcc - tm00 );
    xfloat v[n_ch+1]; // +1 for PWM

    if( UVAR_l ) {  leds[2].set(); }

    v[0] = ina226.getVbus_uV() * (xfloat)1e-6f * v_coeffs[0];
    v[1] = ina226.getI_uA()    * (xfloat)1e-6f * v_coeffs[1];
    v[2] = pwmdat.get_v_real();
    UVAR_z = ina226.get_last_Vsh();

    if( UVAR_l ) {  leds[2].reset(); }

    if( do_out ) {
      std_out <<  FltFmt( tc, cvtff_auto, 12, 4 );
    }

    sdat.add( v );

    if( do_out ) {
      for( auto vc : v ) {
        std_out  << ' '  <<  vc;
      }
      std_out << NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  pwmdat.end_run();

  sdat.calc();
  std_out << sdat << NL;

  delay_ms( 10 );

  return rc;
}


int cmd_set_calibr( int argc, const char * const * argv )
{
  float calibr_I_lsb = arg2float_d( 1, argc, argv, ina226.get_I_lsb_mA()  * 1e-3f, 1e-20f, 1e10f );
  float calibr_R     = arg2float_d( 2, argc, argv, ina226.get_R_sh_uOhm() * 1e-6f, 1e-20f, 1e10f );
  float V_sh_max =  (int)INA226::lsb_V_sh_nv * 1e-9f * 0x7FFF;
  ina226.set_calibr_val( (uint32_t)(calibr_R * 1e6f), (uint32_t)(calibr_I_lsb * 1e3f) );
  std_out << "# calibr_I_lsb= " << calibr_I_lsb << " calibr_R= " << calibr_R
     << " V_sh_max=  " << V_sh_max
     << " I_max= " << ( V_sh_max / calibr_R ) << " / " << ( calibr_I_lsb * 0x7FFF ) << NL;
  return 0;
}


// ------------------------------------------- PWM ---------------------------------------

void tim_cfg()
{
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = UVAR_p;
  tim_h.Init.Period            = UVAR_a;
  tim_h.Init.ClockDivision     = 0;
  tim_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim_h ) != HAL_OK ) {
    UVAR_e = 1; // like error
    return;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_h, &sClockSourceConfig );

  int pbase = UVAR_a;
  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;


  HAL_TIM_PWM_Stop( &tim_h, TIM_CHANNEL_1 );
  tim_oc_cfg.Pulse = pwmdat.get_v_def() * pbase / 100;
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR_e = 11;
    return;
  }
  HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 );

}

int cmd_pwm( int argc, const char * const * argv )
{
  float v = arg2float_d( 1, argc, argv, 10, 0, 100 );
  pwmdat.set_v_manual( v );
  tim_print_cfg( TIM_EXA );
  std_out << NL "PWM:  in: " << pwmdat.get_v() << "  real: " << pwmdat.get_v_real() << NL;
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
  std_out << "# Coefficients: "
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


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

