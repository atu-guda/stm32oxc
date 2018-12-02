#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <algorithm>

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
uint16_t ADC_buf[16];

TIM_HandleTypeDef tim_h;
using tim_ccr_t = decltype( tim_h.Instance->CCR1 );
void tim_cfg();
void pwm_recalc();
void set_pwm(); // uses pwm_val
float pwm_val = 10.0f;
float pwm_min = 10.0f, pwm_max = 50.0f;
float pwm_hand = 0;

const unsigned max_steps = 32;
struct StepInfo {
  float v;
  int t, tp;
};
StepInfo pwms[max_steps];
unsigned n_steps  = 0;
int pwm_t = 0;
int pwm_t_mul = 1;
void reset_steps();
void mk_rect( float v, int t );
void mk_ladder( float dv, int dt, unsigned n_up );
void mk_trap( float v, int t1, int t2, int t3 );
void show_steps();

void handle_keys();



// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test ADC"  };
int cmd_tinit( int argc, const char * const * argv );
CmdInfo CMDINFO_TINIT { "tinit", 'I', cmd_tinit, " - reinit timer"  };
int cmd_pwm( int argc, const char * const * argv );
CmdInfo CMDINFO_PWM { "pwm", 'W', cmd_pwm, " [va] - set PWM value"  };
int cmd_set_minmax( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_MINMAX { "set_minmax", 0, cmd_set_minmax, " pwm_min pwm_max - set PWM limits"  };
int cmd_show_steps( int argc, const char * const * argv );
CmdInfo CMDINFO_SHOW_STEPS { "show_steps", 'S', cmd_show_steps, " - show PWM steps"  };
int cmd_mk_rect( int argc, const char * const * argv );
CmdInfo CMDINFO_MK_RECT { "mk_rect", 0, cmd_mk_rect, " v t - make rectangle steps"  };
int cmd_mk_ladder( int argc, const char * const * argv );
CmdInfo CMDINFO_MK_LADDER { "mk_ladder", 0, cmd_mk_ladder, " v t n_up - make ladder steps"  };
int cmd_mk_trap( int argc, const char * const * argv );
CmdInfo CMDINFO_MK_TRAP { "mk_trap", 0, cmd_mk_trap, " v t1  t2 t3 - make trapzoid steps"  };
int cmd_edit_step( int argc, const char * const * argv );
CmdInfo CMDINFO_EDIT_STEP { "edit_step", 'E', cmd_edit_step, " v t tp - edit given step"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TINIT,
  &CMDINFO_PWM,
  &CMDINFO_SET_MINMAX,
  &CMDINFO_SHOW_STEPS,
  &CMDINFO_MK_RECT,
  &CMDINFO_MK_LADDER,
  &CMDINFO_MK_TRAP,
  &CMDINFO_EDIT_STEP,
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

  UVAR('p') = 0;     // PSC,  - max output freq
  UVAR('a') = 1439;  // ARR, to get 100 kHz with PSC = 0
  UVAR('x') = 30000; // time step in PWM, ms
  UVAR('y') = 5;     // value step in pwm, %
  UVAR('m') = 10;    // number of steps

  reset_steps();

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
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)n_ADC_ch_max );

  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  uint32_t sampl_t_idx = clamp( UVAR('s'), 0, (int)adc_n_sampl_times-1 );
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

  float pwm_val0  = pwms[0].v; pwm_val = pwm_val0; pwm_hand = 0;
  float pwm_dt = pwms[0].t;
  float pwm_k = ( pwms[0].tp == 1 ) ? ( ( pwms[1].v - pwm_val0 ) / pwm_dt ): 0;
  pwm_t = 0;
  unsigned step_n = 0;

  int rc = 0;
  uint32_t tm0, tm00;

  for( unsigned i=0; i<n && !break_flag ; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    handle_keys();

    if( pwm_t >= pwm_dt ) { // next step
      pwm_t = 0;
      ++step_n;
      if( step_n >= n_steps ) {
        break;
      }
      pwm_val0  = pwms[step_n].v; pwm_val = pwm_val0;
      pwm_dt = pwms[step_n].t;
      pwm_k = ( pwms[step_n].tp == 1 ) ? ( ( pwms[step_n+1].v - pwm_val0 ) / pwm_dt ): 0;
    }

    float pwm_old = pwm_val;
    pwm_val = pwm_val0 + pwm_k * pwm_t;
    // calc
    if( abs( pwm_old - pwm_val ) > 0.01 ) {
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

  pwm_val = pwm_min;
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
  tim_oc_cfg.Pulse = (tim_ccr_t)( pwm_min * pbase / 100 );
  if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }
  HAL_TIM_PWM_Start( &tim_h, TIM_CHANNEL_1 );

}

int cmd_pwm( int argc, const char * const * argv )
{
  pwm_val = arg2float_d( 1, argc, argv, 10, 1, 100 );
  STDOUT_os;
  set_pwm();
  tim_print_cfg( TIM_EXA );
  os << NL "PWM:  " << pwm_val << NL;
  return 0;
}

void set_pwm()
{
  if( pwm_val > pwm_max ) { pwm_val = pwm_max; };
  if( pwm_val < pwm_min ) { pwm_val = pwm_min; };
  uint32_t scl = tim_h.Instance->ARR;
  tim_h.Instance->CCR1 = (tim_ccr_t)( pwm_val * scl / 100 );
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
    case 'w': pwm_hand += 1; need_set_pwm = true; break;
    case 'W': pwm_hand += 5; need_set_pwm = true; break;
    case 's': pwm_hand -= 1; need_set_pwm = true; break;
    case 'S': pwm_hand -= 5; need_set_pwm = true; break;
    case 'z': pwm_hand  = 0; need_set_pwm = true; break;
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

void reset_steps()
{
  for( auto &s : pwms ) {
    s.v = pwm_min; s.t = 30000; s.tp = 0;
  }
  n_steps = 3;
}

void mk_rect( float v, int t )
{
  pwms[0].v = pwm_min; pwms[0].t = 10000; pwms[0].tp = 0;
  pwms[1].v = v;       pwms[1].t = t;     pwms[1].tp = 0;
  pwms[2].v = pwm_min; pwms[2].t = 60000; pwms[2].tp = 0;
  n_steps = 3;
}

void mk_ladder( float dv, int t, unsigned n_up )
{
  unsigned n_up_max = max_steps / 2 - 1;
  n_up = clamp( n_up, 1u, n_up_max );

  pwms[0].v = pwm_min; pwms[0].t = 10000; pwms[0].tp = 0;
  unsigned i = 1;
  float cv = pwm_min;
  for( /* NOP */; i <= n_up; ++i ) {
    cv += dv;
    pwms[i].v = cv;
    pwms[i].t = t; pwms[i].tp = 0;
  }
  for( /* NOP */; i < n_up*2; ++i ) {
    cv -= dv;
    pwms[i].v = cv;
    pwms[i].t = t; pwms[i].tp = 0;
  }
  pwms[i].v = pwm_min; pwms[i].t = 60000; pwms[0].tp = 0;
  n_steps = n_up * 2 + 1;
}

void mk_trap( float v, int t1, int t2, int t3 )
{
  pwms[0].v = pwm_min; pwms[0].t = 10000; pwms[0].tp = 0;
  pwms[1].v = pwm_min; pwms[1].t = t1;    pwms[1].tp = 1;
  pwms[2].v = v;       pwms[2].t = t2;    pwms[2].tp = 0;
  pwms[3].v = v;       pwms[3].t = t3;    pwms[3].tp = 1;
  pwms[4].v = pwm_min; pwms[4].t = 60000; pwms[4].tp = 0;
  n_steps = 5;
}

void show_steps()
{
  STDOUT_os;
  os << "# pwm_min= " << pwm_min << "  pwm_max= " << pwm_max << " n_steps= " << n_steps << NL;
  int tc = 0;
  for( unsigned i=0; i<n_steps; ++i ) {
    os << '[' << i << "] " << tc << ' ' << pwms[i].t << ' ' << pwms[i].v << ' ' << pwms[i].tp << NL;
    tc += pwms[i].t;
  }
}

int cmd_show_steps( int /*argc*/, const char * const * /*argv*/ )
{
  show_steps();
  return 0;
}

int cmd_set_minmax( int argc, const char * const * argv )
{
  pwm_min = arg2float_d( 1, argc, argv, 10, 1, 98 );
  pwm_max = arg2float_d( 2, argc, argv, 50, pwm_min+1, 99 );
  return 0;
}

int cmd_mk_rect( int argc, const char * const * argv )
{
  float v  = arg2float_d( 1, argc, argv,    35, 1, 98 );
  int   t  = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  mk_rect( v, t );
  show_steps();
  return 0;
}

int cmd_mk_ladder( int argc, const char * const * argv )
{
  float v      = arg2float_d( 1, argc, argv,    5,  1, 90 );
  unsigned  t  = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  unsigned  n  = arg2long_d(  3, argc, argv,     8, 1, max_steps/2-2 );
  mk_ladder( v, t, n );
  show_steps();
  return 0;
}

int cmd_mk_trap( int argc, const char * const * argv )
{
  float v  = arg2float_d( 1, argc, argv,    30, 0, 98 );
  int   t1 = arg2long_d(  2, argc, argv, 30000, 1, 10000000 );
  int   t2 = arg2long_d(  3, argc, argv, 30000, 1, 10000000 );
  int   t3 = arg2long_d(  4, argc, argv, 30000, 1, 10000000 );
  mk_trap( v, t1, t2, t3 );
  show_steps();
  return 0;
}

int cmd_edit_step( int argc, const char * const * argv )
{
  unsigned j = arg2long_d(1, argc, argv,     0, 0, max_steps-1 );
  float v  = arg2float_d( 2, argc, argv,    25, 0, 99 );
  int   t  = arg2long_d(  3, argc, argv, 30000, 1, 10000000 );
  int   tp = arg2long_d(  4, argc, argv,     0, 0, 1 );
  pwms[j].v = v; pwms[j].t = t; pwms[j].tp = tp;
  if( j >= n_steps ) {
    n_steps = j+1;
  }
  show_steps();
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

