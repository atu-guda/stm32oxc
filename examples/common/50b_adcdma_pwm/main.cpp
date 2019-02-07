#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cerrno>

#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>

#include <../examples/common/inc/pwm1_ctl.h>

using namespace std;
using namespace SMLRL;

using sreal = StatData::sreal;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure via inner ADC with PWM control" NL;

int adc_init_exa_4ch_manual( ADC_Info &adc, uint32_t adc_presc, uint32_t sampl_cycl, uint8_t n_ch );

ADC_Info adc;

int v_adc_ref = BOARD_ADC_COEFF; // in mV, measured before test, adjust as UVAR('v')
uint16_t ADC_buf[32];

TIM_HandleTypeDef tim_h;
using tim_ccr_t = decltype( tim_h.Instance->CCR1 );
void tim_cfg();


PWMData pwmdat( tim_h );

void handle_keys();



// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] [skip_pwm] - measure ADC + control PWM"  };
int cmd_tinit( int argc, const char * const * argv );
CmdInfo CMDINFO_TINIT { "tinit", 'I', cmd_tinit, " - reinit timer"  };
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", 'W', cmd_pwm, " [val] - set PWM value"  };
int cmd_set_coeffs( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_COEFFS { "set_coeffs", 'F', cmd_set_coeffs, " k0 k1 k2 k3 - set ADC coeffs"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TINIT,
  &CMDINFO_PWM,
  CMDINFOS_PWM,
  &CMDINFO_SET_COEFFS,
  nullptr
};

const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
float v_coeffs[n_ADC_ch_max] = { 1.0f, 1.0f, 1.0f, 1.0f };


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10; // 10 ms
  UVAR('v') = v_adc_ref;
  UVAR('c') = 2; // n_ADC_ch_max;
  UVAR('n') = 1000000; // number of series (10ms 't' each): limited by steps
  UVAR('s') = 6; // sampling time index

  UVAR('p') = 0;     // PSC,  - max output freq
  UVAR('a') = 1439;  // ARR, to get 100 kHz with PSC = 0

  #ifdef PWR_CR1_ADCDC1
  // PWR->CR1 |= PWR_CR1_ADCDC1;
  #endif

  tim_cfg();

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
  STDOUT_os;
  uint32_t t_step = UVAR('t');
  uint32_t n_ch = clamp( UVAR('c'), 1, (int)n_ADC_ch_max );

  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  bool skip_pwm = arg2long_d( 2, argc, argv, 0, 1, 1 ); // dont touch PWM

  StatData sdat( n_ch );

  uint32_t sampl_t_idx = clamp( UVAR('s'), 0, (int)adc_n_sampl_times-1 );
  uint32_t f_sampl_max = adc.adc_clk / ( sampl_times_cycles[sampl_t_idx] * n_ch );

  uint32_t adc_presc = hint_ADC_presc();
  UVAR('i') =  adc_init_exa_4ch_manual( adc, adc_presc, sampl_times_codes[sampl_t_idx], n_ch );
  delay_ms( 1 );
  if( ! UVAR('i') ) {
    os << "ADC init failed, errno= " << errno << NL;
    return 1;
  }
  if( UVAR('d') > 1 ) { pr_ADC_state( adc );  }


  int div_val = -1;
  adc.adc_clk = calc_ADC_clk( adc_presc, &div_val );
  os << "# ADC: n_ch= " << n_ch << " n= " << n << " adc_clk= " << adc.adc_clk << " div_val= " << div_val
     << " s_idx= " << sampl_t_idx << " sampl= " << sampl_times_cycles[sampl_t_idx]
     << " f_sampl_max= " << f_sampl_max << " Hz" NL;
  delay_ms( 10 );

  uint32_t n_ADC_sampl = n_ch;

  sreal kv = 0.001f * UVAR('v') / 4096;

  adc.reset_cnt();

  os << "# n = " << n << " n_ch= " << n_ch << " t_step= " << t_step << NL;
  os << "# skip_pwm= " << skip_pwm << NL << "# Coeffs: ";
  for( decltype(n_ch) j=0; j<n_ch; ++j ) {
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
    sreal v[n_ch+1]; // +1 for PWM

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    adc.end_dma = 0;
    if( HAL_ADC_Start_DMA( &adc.hadc, (uint32_t*)(&ADC_buf), n_ADC_sampl ) != HAL_OK )   {
      os <<  "ADC_Start_DMA error" NL;
      rc = 1;
      break;
    }

    for( uint32_t ti=0; adc.end_dma == 0 && ti<5000; ++ti ) { // 11
      delay_mcs( 2 );
    }

    HAL_ADC_Stop_DMA( &adc.hadc ); // needed
    if( UVAR('l') ) {  leds.reset( BIT2 ); }
    if( adc.end_dma == 0 ) {
      os <<  "Fail to wait DMA end " NL;
      rc = 2;
      break;
    }
    if( adc.dma_error != 0 ) {
      os <<  "Found DMA error " << HexInt( adc.dma_error ) <<  NL;
      rc = 3;
      break;
    } else {
      adc.n_series = 1;
    }

    if( do_out ) {
      os <<  FltFmt( tc, cvtff_auto, 12, 4 );
    }
    for( decltype(n_ch) j=0; j<n_ch; ++j ) {
      sreal cv = kv * ADC_buf[j] * v_coeffs[j];
      v[j] = cv;
    }
    v[n_ch] = pwmdat.get_v_real();
    sdat.add( v );

    if( do_out ) {
      for( auto vc : v ) {
        os  << ' '  << vc;
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



void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  adc.end_dma |= 1;
  adc.good_SR =  adc.last_SR = adc.hadc.Instance->SR;
  adc.last_end = 1;
  adc.last_error = 0;
  ++adc.n_good;
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  adc.end_dma |= 2;
  adc.bad_SR = adc.last_SR = adc.hadc.Instance->SR;
  // tim2_deinit();
  adc.last_end  = 2;
  adc.last_error = HAL_ADC_GetError( hadc );
  adc.dma_error = hadc->DMA_Handle->ErrorCode;
  hadc->DMA_Handle->ErrorCode = 0;
  ++adc.n_bad;
}

void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler( &adc.hdma_adc );
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
  tim_oc_cfg.Pulse = (tim_ccr_t)( pwmdat.get_v_def() * pbase / 100 );
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }
  HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 );

}

int cmd_pwm( int argc, const char * const * argv )
{
  float v = arg2float_d( 1, argc, argv, 10, 0, 100 );
  STDOUT_os;
  pwmdat.set_v_manual( v );
  tim_print_cfg( TIM_EXA );
  os << NL "PWM:  in: " << pwmdat.get_v() << "  real: " << pwmdat.get_v_real() << NL;
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


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

