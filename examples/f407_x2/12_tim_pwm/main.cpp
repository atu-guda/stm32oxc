#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

TIM_HandleTypeDef tim_h;
int pwm_vals[] = { 25, 50, 75, 90 };
void tim_cfg();
void pwm_recalc();
void pwm_update();
void pwm_print_cfg();

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


extern "C" {
void task_main( void *prm UNUSED_ARG );
}




int main(void)
{
  BOARD_PROLOG;


  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('p') = 16799;// prescaler, 168MHz->10kHz
  UVAR('a') = 9999; // ARR, 10kHz->1Hz
  UVAR('r') = 0;    // flag: raw values


  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  tim_cfg();

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  for( int i=0; i<4; ++i ) {
    if( argc > i+1 ) {
      pwm_vals[i] = strtol( argv[i+1], 0, 0 );
    }
  }

  pr( NL "Test0: pwm_vals[]= " );
  for( int i=0; i<4; ++i ) {
    pr_d( pwm_vals[i] ); pr( " " );
  }
  pr( NL );

  pwm_print_cfg();
  // pwm_recalc();
  pwm_update();

  return 0;
}

int cmd_tinit( int argc, const char * const * argv )
{
  pwm_print_cfg();
  tim_cfg();

  return 0;
}

//  ----------------------------- configs ----------------

void tim_cfg()
{
  tim_h.Instance = TIM_EXA;
  tim_h.Init.Prescaler = UVAR('p');
  tim_h.Init.Period    = UVAR('a');
  tim_h.Init.ClockDivision = 0;
  tim_h.Init.CounterMode = TIM_COUNTERMODE_UP;
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
  if( UVAR('r') ) { // raw values
    scl = 100;
  }
  tim_h.Instance->CCR1 = pwm_vals[0] * scl / 100;
  tim_h.Instance->CCR2 = pwm_vals[1] * scl / 100;
  tim_h.Instance->CCR3 = pwm_vals[2] * scl / 100;
  tim_h.Instance->CCR4 = pwm_vals[3] * scl / 100;
}

void pwm_print_cfg()
{
  int presc = UVAR('p');
  int arr   = UVAR('a');
  uint32_t hclk  = HAL_RCC_GetHCLKFreq();
  uint32_t pclk2 = HAL_RCC_GetPCLK2Freq(); // for TIM1
  if( hclk != pclk2 ) { // *2 : if APB2 prescaler != 1 (=2)
    pclk2 *= 2;
  }

  int freq1 = pclk2  / ( presc + 1 );
  int freq2 = freq1 / ( arr + 1 );
  pr( NL TIM_EXA_STR " reinit: prescale: " ); pr_d( presc );
  pr( " ARR: " ); pr_d( arr );
  pr( " freq1: " ); pr_d( freq1 );
  pr( " freq2: " ); pr_d( freq2 ); pr( NL );
  pr( "CCR1: " );   pr_d( tim_h.Instance->CCR1 ); pr( NL );
}

//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

