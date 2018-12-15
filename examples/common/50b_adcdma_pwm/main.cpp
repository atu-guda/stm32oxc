#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
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
float pwm_val = 10.0f;     // unrestricted
float pwm_val_1 = pwm_val; // w/o hand
float pwm_val_r = pwm_val; // real
float pwm_min = 10.0f, pwm_max = 50.0f;
float pwm_hand = 0;       // adjusted by hand (handle_keys)

const unsigned max_steps = 32;
struct StepInfo {
  float v;
  int t, tp;
};
StepInfo pwms[max_steps];
unsigned n_steps  = 0;
int pwm_t = 0;
float pwm_t_mul = 1;
void reset_steps();
void mk_rect( float v, int t );
void mk_ladder( float dv, int dt, unsigned n_up );
void mk_trap( float v, int t1, int t2, int t3 );
void show_steps();

void handle_keys();



// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] [skip_pwm] - measure ADC + control PWM"  };
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
  UVAR('n') = 1000000; // number of series (10ms 't' each): limited by steps
  UVAR('s') = 6; // sampling time index

  UVAR('p') = 0;     // PSC,  - max output freq
  UVAR('a') = 1439;  // ARR, to get 100 kHz with PSC = 0

  reset_steps();

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
  int t_step = UVAR('t');
  uint8_t n_ch = clamp( UVAR('c'), 1, (int)n_ADC_ch_max );

  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  bool skip_pwm = arg2long_d( 2, argc, argv, 0, 1, 1 ); // dont touch PWM

  double adc_min[n_ADC_ch_max], adc_max[n_ADC_ch_max], adc_mean[n_ADC_ch_max],
        adc_sum[n_ADC_ch_max], adc_sum2[n_ADC_ch_max];
  for( unsigned j=0; j<n_ADC_ch_max; ++j ) {
    adc_min[j] = 5.1e37; adc_max[j] = -5.1e37; adc_mean[j] = 0;
    adc_sum[j] = adc_sum2[j] = 0;
  }
  unsigned adc_n = 0;

  os << "n = " << n << " n_ch= " << n_ch << " skip_pwm= " << skip_pwm << " t_step= " << t_step << NL;

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

  float pwm_val0  = pwms[0].v; pwm_val = pwm_val_1 = pwm_val0; pwm_hand = 0;
  float pwm_dt = pwms[0].t;
  float pwm_k = ( pwms[0].tp == 1 ) ? ( ( pwms[1].v - pwm_val0 ) / pwm_dt ): 0;
  if( ! skip_pwm ) {
    pwm_val_r = pwm_val;
  }
  pwm_t = 0;
  pwm_t_mul = 1;
  unsigned step_n = 0;

  int rc = 0;
  uint32_t tm0, tm00;

  for( unsigned i=0; i<n && !break_flag ; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    handle_keys();

    if( ! skip_pwm ) {
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

      pwm_val_1 = pwm_val0 + pwm_k * pwm_t;
      pwm_val = pwm_val_1 + pwm_hand;
      set_pwm();
    }

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    adc.end_dma = 0;
    if( HAL_ADC_Start_DMA( &adc.hadc, (uint32_t*)(&ADC_buf), n_ADC_sampl ) != HAL_OK )   {
      os <<  "ADC_Start_DMA error" NL;
      rc = 1;
      break;
    }

    for( uint32_t ti=0; adc.end_dma == 0 && ti<5000; ++ti ) {
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

    int dt = tcc - tm00; // ms
    os <<  FloatFmt( 0.001f * dt, "%-10.4f "  );
    UVAR('z') = ADC_buf[0];
    for( int j=0; j<n_ch; ++j ) {
      double cv = ( 0.001f * UVAR('v')  * ADC_buf[j] / 4096 );
      if( cv < adc_min[j] ) { adc_min[j] = cv; }
      if( cv > adc_max[j] ) { adc_max[j] = cv; }
      adc_sum[j]  += cv;
      adc_sum2[j] += cv * cv;
      os << ' ' << cv;
    }

    UVAR('x') = step_n;
    UVAR('y') = pwm_t;
    if( UVAR('d') > 0 ) {
      os << ' ' << pwm_val_r <<  ' ' << ' ' << pwm_val << ' ' << pwm_hand << ' ' << pwm_t << ' ' << step_n << NL;
    } else {
      os << ' ' << pwm_val_r <<  NL;
    }

    pwm_t += t_step * pwm_t_mul;
    ++adc_n;
    delay_ms_until_brk( &tm0, t_step );
  }

  if( ! skip_pwm ) {
    pwm_val = pwm_min; pwm_hand = 0; pwm_t = 0; step_n = 0;
    set_pwm();
  }

  os << NL "# n_real= " << adc_n;
  os << NL "# mean ";
  for( int j=0; j<n_ch; ++j ) {
    adc_mean[j] = adc_sum[j] / adc_n;
    os << ' ' << adc_mean[j];
  }
  os << NL "# min  ";
  for( int j=0; j<n_ch; ++j ) {
    os << ' ' << adc_min[j];
  }
  os << NL "# max  ";
  for( int j=0; j<n_ch; ++j ) {
    os << ' ' << adc_max[j];
  }
  os << NL "# sum  ";
  for( int j=0; j<n_ch; ++j ) {
    os << ' ' << adc_sum[j];
  }
  os << NL "# sum2 ";
  for( int j=0; j<n_ch; ++j ) {
    os << ' ' << adc_sum2[j];
  }
  os << NL "# sd  ";
  for( int j=0; j<n_ch; ++j ) {
    os << ' ' << (sqrt(  adc_sum2[j] * adc_n - adc_sum[j] * adc_sum[j] ) / adc_n );
  }
  os << NL;


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
  pwm_hand = 0;
  STDOUT_os;
  set_pwm();
  tim_print_cfg( TIM_EXA );
  os << NL "PWM:  in: " << pwm_val << "  real: " << pwm_val_r << NL;
  return 0;
}

void set_pwm()
{
  pwm_val_r = clamp( pwm_val, pwm_min, pwm_max );
  uint32_t scl = tim_h.Instance->ARR;
  tim_ccr_t nv = (tim_ccr_t)( pwm_val_r * scl / 100 );
  if( nv != tim_h.Instance->CCR1 ) {
    tim_h.Instance->CCR1 =nv;
  }
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
    case 'w': pwm_hand += 1;  break;
    case 'W': pwm_hand += 5;  break;
    case 's': pwm_hand -= 1;  break;
    case 'S': pwm_hand -= 5;  break;
    case 'z': pwm_hand  = 0;  break;
    case '0': pwm_hand =    - pwm_val_1;  break;
    case '1': pwm_hand = 10 - pwm_val_1;  break;
    case '2': pwm_hand = 20 - pwm_val_1;  break;
    case '3': pwm_hand = 30 - pwm_val_1;  break;
    case '4': pwm_hand = 40 - pwm_val_1;  break;
    case '5': pwm_hand = 50 - pwm_val_1;  break;
    case 'a': pwm_t    -=  2000;  break;
    case 'A': pwm_t    -= 10000;  break;
    case 'd': pwm_t    +=  2000;  break;
    case 'D': pwm_t    += 10000;  break;
    case 'g': pwm_t_mul = 0; break;
    case 'G': pwm_t_mul = 1; break;
    default: break;
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
  pwms[2].v = pwm_min; pwms[2].t = 30000; pwms[2].tp = 0;
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
  pwms[4].v = pwm_min; pwms[4].t = 30000; pwms[4].tp = 0;
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
  os << "# Total: " << tc << " ms" NL;
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

