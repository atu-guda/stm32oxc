#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <vector>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;




int adc_init_exa_4ch_manual( ADC_Info &adc, uint32_t adc_presc, uint32_t sampl_cycl, uint8_t n_ch );

ADC_Info adc;

int v_adc_ref = BOARD_ADC_COEFF; // in mV, measured before test, adjust as UVAR('v')
const uint32_t n_ADC_ch_max = 4; // current - in UVAR('c')
uint16_t ADC_buf[32];

TIM_HandleTypeDef tim_h;
void tim_cfg();
void pwm_recalc();
void set_pwm(); // uses pwm_val
int pwm_val = 10;
int pwm_t = 0;
int pwm_t_mul = 1;

void handle_keys();



const int pbufsz = 128;
// FIL out_file;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test ADC"  };
int cmd_tinit( int argc, const char * const * argv );
CmdInfo CMDINFO_TINIT { "tinit", 'I', cmd_tinit, " - reinit timer"  };
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", 'W', cmd_pwm, " [va] - set PWM value"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TINIT,
  &CMDINFO_PWM,
  nullptr
};



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10; // 10 ms
  UVAR('v') = v_adc_ref;
  UVAR('c') = 2; // n_ADC_ch_max;
  UVAR('n') = 50000; // number of series (10ms 't' each)
  UVAR('s') = 6; // sampling time index

  UVAR('a') = 1439;  // ARR, to get 100 kHz with PSC = 0
  UVAR('p') = 0;     // PSC,  - max optput freq
  UVAR('x') = 30000; // time step in PWM, ms
  UVAR('y') = 5;     // value step in pwm, %
  UVAR('z') = 45;    // max value in pwm, %
  UVAR('l') = 10;    // start value in pwm, %
  UVAR('m') = 10;    // number of steps
  pwm_val = UVAR('l');

  #ifdef PWR_CR1_ADCDC1
  PWR->CR1 |= PWR_CR1_ADCDC1;
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
  int t_step = UVAR('t');
  uint8_t n_ch = UVAR('c');
  if( n_ch > n_ADC_ch_max ) { n_ch = n_ADC_ch_max; };
  if( n_ch < 1 ) { n_ch = 1; };

  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  uint32_t sampl_t_idx = UVAR('s');
  if( sampl_t_idx >= adc_n_sampl_times ) { sampl_t_idx = adc_n_sampl_times-1; };
  uint32_t f_sampl_max = adc.adc_clk / ( sampl_times_cycles[sampl_t_idx] * n_ch );

  uint32_t adc_presc = hint_ADC_presc();
  UVAR('i') =  adc_init_exa_4ch_manual( adc, adc_presc, sampl_times_codes[sampl_t_idx], n_ch );
  delay_ms( 1 );
  if( ! UVAR('i') ) {
    os <<  "ADC init failed, errno= " << errno << NL;
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

  adc.reset_cnt();

  leds.reset( BIT0 | BIT1 | BIT2 );

  pwm_val  = UVAR('l');
  int pwm_step = UVAR('y');
  pwm_t = 0;
  int pwm_n = 0;
  uint32_t tm0, tm00;
  int rc = 0;

  for( unsigned i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    handle_keys();

    if( pwm_t >= UVAR('x') ) {
      pwm_t = 0;
      ++pwm_n;
      pwm_val += pwm_step;
      if( pwm_n == UVAR('m') ) {
        pwm_step = - pwm_step;
      }
      set_pwm();
    }

    adc.end_dma = 0;
    if( HAL_ADC_Start_DMA( &adc.hadc, (uint32_t*)(&ADC_buf), n_ADC_sampl ) != HAL_OK )   {
      os <<  "ADC_Start_DMA error" NL;
      rc = 1;
      break;
    }

    for( uint32_t ti=0; adc.end_dma == 0 && ti<1000; ++ti ) {
      delay_mcs( 10 );
    }

    HAL_ADC_Stop_DMA( &adc.hadc ); // needed
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

    int dt = tcc - tm00; // ms
    os <<  FloatFmt( 0.001f * dt, "%-10.4f "  );
    for( int j=0; j<n_ch; ++j ) {
      os << ' ' << ( 0.001f * UVAR('v')  * ADC_buf[j] / 4096 );
    }
    os << ' ' << pwm_val <<  NL;

    pwm_t += t_step * pwm_t_mul;
    delay_ms_until_brk( &tm0, t_step );
  }

  pwm_val = UVAR('l');
  set_pwm();


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

  pwm_recalc();
}

void pwm_recalc()
{
  int pbase = UVAR('a');
  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;


  HAL_TIM_PWM_Stop( &tim_h, TIM_CHANNEL_1 );
  tim_oc_cfg.Pulse = UVAR('l') * pbase / 100;
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }
  HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 );

}

int cmd_pwm( int argc, const char * const * argv )
{
  pwm_val = arg2long_d( 1, argc, argv, 10, 1, 100 ); // number of series
  STDOUT_os;
  set_pwm();
  tim_print_cfg( TIM_EXA );
  os << NL "PWM:  " << pwm_val << NL;
  return 0;
}

void set_pwm()
{
  if( pwm_val > UVAR('z') ) { pwm_val = UVAR('z'); };
  if( pwm_val < UVAR('l') ) { pwm_val = UVAR('l'); };
  uint32_t scl = tim_h.Instance->ARR;
  tim_h.Instance->CCR1 = pwm_val * scl / 100;
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
  bool need_set_pwm = false;

  switch( v.c ) {
    case 'w': pwm_val += 1; need_set_pwm = true; break;
    case 'W': pwm_val += 5; need_set_pwm = true; break;
    case 's': pwm_val -= 1; need_set_pwm = true; break;
    case 'S': pwm_val -= 5; need_set_pwm = true; break;
    case 'a': pwm_t   -=  2000;  break;
    case 'A': pwm_t   -= 10000;  break;
    case 'd': pwm_t   +=  2000;  break;
    case 'D': pwm_t   += 10000;  break;
    case '0': pwm_val =  0; need_set_pwm = true; break;
    case '1': pwm_val = 10; need_set_pwm = true; break;
    case '2': pwm_val = 20; need_set_pwm = true; break;
    case '3': pwm_val = 30; need_set_pwm = true; break;
    case '4': pwm_val = 40; need_set_pwm = true; break;
    case '5': pwm_val = 50; need_set_pwm = true; break;
    case 'g': pwm_t_mul = 0; break;
    case 'G': pwm_t_mul = 1; break;
    default: break;
  }

  if( need_set_pwm ) {
    set_pwm();
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

