#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_tim.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

TIM_HandleTypeDef tim_h;
int pwm_vals[] = { 25, 50, 75, 90 };
void tim_cfg();
void pwm_update();

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 10;
  UVAR('n') = 1000;
  UVAR('p') = calc_TIM_psc_for_cnt_freq( TIM_EXA, 1000000  ); // ->1 MHz
  UVAR('a') = 999; // ARR, 1 MHz -> 1 kHz
  UVAR('m') = 0;    // mode: 0: up, 1: down, 2: updown

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
  for( int i=0; i<4; ++i ) {
    if( argc > i+1 ) {
      pwm_vals[i] = strtol( argv[i+1], 0, 0 );
    }
  }

  STDOUT_os;
  os << NL "Test0: pwm_vals[]= ";
  for( int i=0; i<4; ++i ) {
    os << pwm_vals[i] <<  ' ';
  }
  os <<  NL ;

  tim_print_cfg( TIM_EXA );
  // pwm_recalc();
  pwm_update();

  return 0;
}

//  ----------------------------- configs ----------------

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

  const int nch = 4;
  const int channels[nch] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };

  for( int i=0; i<nch; ++i ) {
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
  tim_h.Instance->CCR1 = pwm_vals[0] * scl / 1000;
  tim_h.Instance->CCR2 = pwm_vals[1] * scl / 1000;
  tim_h.Instance->CCR3 = pwm_vals[2] * scl / 1000;
  tim_h.Instance->CCR4 = pwm_vals[3] * scl / 1000;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

