#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_tim.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test timer as PWM source" NL;

TIM_HandleTypeDef tim_h;
const unsigned n_pwm_ch = 4;
int pwm_vals[n_pwm_ch] = { 25, 50, 75, 90 };
void tim_cfg();
void pwm_recalc();
void pwm_update();

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_tinit( int argc, const char * const * argv );
CmdInfo CMDINFO_TINIT { "tinit", 'I', cmd_tinit, " - reinit timer"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TINIT,
  nullptr
};

const unsigned n_countmodes = 5;
const uint32_t countmodes[n_countmodes] = { TIM_COUNTERMODE_UP, TIM_COUNTERMODE_DOWN, TIM_COUNTERMODE_CENTERALIGNED1,
 TIM_COUNTERMODE_CENTERALIGNED2, TIM_COUNTERMODE_CENTERALIGNED3 };


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM_EXA, 10000  ); // ->10kHz
  UVAR('a') = 9999; // ARR, 10kHz->1Hz
  UVAR('r') = 0;    // flag: raw values
  UVAR('m') = 0;    // mode: 0: up, 1: down, 2: updown
  UVAR('o') = 0;    // pOlarity 0: high 1: low

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  tim_cfg();

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  for( decltype(+n_pwm_ch) i=0; i<n_pwm_ch; ++i ) {
    if( argc <= (int)(i+1) ) {
      break;
    }
    pwm_vals[i] = strtol( argv[i+1], 0, 0 );
  }

  STDOUT_os;
  os << NL "# Test0: pwm_vals[]= ";
  for( auto v : pwm_vals ) {
    os << v <<  ' ';
  }
  os <<  NL ;

  tim_print_cfg( TIM_EXA );
  // pwm_recalc();
  pwm_update();

  return 0;
}

int cmd_tinit( int argc, const char * const * argv )
{
  tim_cfg();
  tim_print_cfg( TIM_EXA );

  return 0;
}

//  ----------------------------- configs ----------------

void tim_cfg()
{
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Prescaler         = UVAR('p');
  tim_h.Init.Period            = UVAR('a');
  tim_h.Init.ClockDivision     = 0;
  unsigned cmode = UVAR('m');
  if( cmode > n_countmodes ) {
    cmode = 0;
  }
  tim_h.Init.CounterMode       = countmodes[cmode];
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
  tim_oc_cfg.OCPolarity   = UVAR('o') ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  const decltype(TIM_CHANNEL_1) channels[n_pwm_ch] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };

  for( unsigned i=0; i<n_pwm_ch; ++i ) {
    HAL_TIM_PWM_Stop( &tim_h, channels[i] );
    tim_oc_cfg.Pulse = pwm_vals[i] * pbase / 100;
    if( HAL_TIM_PWM_ConfigChannel( &tim_h, &tim_oc_cfg, channels[i] ) != HAL_OK ) {
      UVAR('e') = 11+i;
      return;
    }
    HAL_TIM_PWM_Start( &tim_h, channels[i] );
  }

}

void pwm_update()
{
  tim_h.Instance->PSC  = UVAR('p');
  int pbase = UVAR('a');
  tim_h.Instance->ARR  = pbase;
  int scl = pbase;
  if( UVAR('r') ) { // raw values
    scl = 100;
  }
  tim_h.Instance->CCR1 = pwm_vals[0] * scl / 100;
  tim_h.Instance->CCR2 = pwm_vals[1] * scl / 100;
  tim_h.Instance->CCR3 = pwm_vals[2] * scl / 100;
  tim_h.Instance->CCR4 = pwm_vals[3] * scl / 100;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

