#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <oxc_ads1115.h>

#include <../examples/common/inc/pwm1_ctl.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test ADS1115 I2C ADC device with  PWM control" NL;

TIM_HandleTypeDef tim_h;
using tim_ccr_t = decltype( tim_h.Instance->CCR1 );
void tim_cfg();


PWMData pwmdat( tim_h );

void handle_keys();



// --- local commands;
DCL_CMD_REG( test0, 'T', " [n] [skip_pwm] - measure ADC + control PWM"  );
DCL_CMD_REG( tinit, 'I', " - reinit timer"  );
DCL_CMD_REG( pwm, 'W', " [val] - set PWM value"  );
DCL_CMD_REG( set_coeffs, 'F', " k0 k1 k2 k3 - set ADC coeffs"  );


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
ADS1115 adc( i2cd );
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
xfloat v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };


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
  i2c_client_def = &adc;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR('t');
  uint32_t n_ch = clamp( UVAR('c'), 1, (int)n_ADC_ch_max );

  uint8_t e_ch = (uint8_t)(n_ch-1);

  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  bool skip_pwm = arg2long_d( 2, argc, argv, 0, 1, 1 ); // dont touch PWM

  StatData sdat( n_ch );

  adc.setDefault();

  uint16_t cfg =  ADS1115::cfg_pga_4096 | ADS1115::cfg_rate_860 | ADS1115::cfg_oneShot;
  UVAR('e') = adc.setCfg( cfg );
  uint16_t x_cfg = adc.getDeviceCfg();
  int scale_mv = adc.getScale_mV();
  std_out <<  "# cfg= " << HexInt16( x_cfg ) << " scale_mv = " << scale_mv << NL;

  int16_t ADC_buf[n_ADC_ch_max];
  xfloat kv = 0.001f * scale_mv / 0x7FFF;

  std_out << "# n = " << n << " n_ch= " << n_ch << " t_step= " << t_step << NL;
  std_out << "# skip_pwm= " << skip_pwm << NL << "# Coeffs: ";
  for( decltype(n_ch) j=0; j<n_ch; ++j ) {
    std_out << ' ' << v_coeffs[j];
  }
  std_out << NL;

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  pwmdat.prep( t_step, skip_pwm );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR('b');

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

    float tc = 0.001f * ( tcc - tm00 );
    xfloat v[n_ch+1]; // +1 for PWM

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    unsigned no = adc.getOneShotNch( 0, e_ch, ADC_buf );
    if( UVAR('l') ) {  leds.reset( BIT2 ); }
    if( no != n_ch ) {
      std_out << "# Error: read only " << no << " channels" << NL;
      break;
    }

    if( do_out ) {
      std_out <<  FltFmt( tc, cvtff_auto, 12, 4 );
    }
    for( decltype(n_ch) j=0; j<n_ch; ++j ) {
      xfloat cv = kv * ADC_buf[j] * v_coeffs[j];
      v[j] = cv;
    }
    v[n_ch] = pwmdat.get_v_real();
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
  tim_oc_cfg.Pulse = pwmdat.get_v_def() * pbase / 100;
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }
  HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 );

}

int cmd_pwm( int argc, const char * const * argv )
{
  float v = arg2float_d( 1, argc, argv, 10.0f, 0.0f, 100.0f );
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

