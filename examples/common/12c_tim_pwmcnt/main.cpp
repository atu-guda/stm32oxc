#include <cerrno>

#include <oxc_auto.h>
#include <oxc_main.h>

#include "tim_cnt.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test timer as ???" NL;

TIM_HandleTypeDef tim_pwm_h;
TIM_HandleTypeDef tim_cnt_h;
int MX_TIM_PWM_Init();
int MX_TIM_IN_Init();

// --- local commands;
DCL_CMD_REG( test0, 'T', " [t] [n] - test counter? "  );
DCL_CMD_REG( tinit, 'I', " - reinit timer"  );
DCL_CMD_REG( set_pwm, 'W', " v - set PWM value in %"  );



int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 5000;
  UVAR_n = 20;
  UVAR_p = calc_TIM_psc_for_cnt_freq( TIM_EXA, 10000  ); // ->10kHz
  UVAR_a = 99; // ARR, 10kHz->100Hz

  UVAR_s = 5;  // step in pwm

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  // tim_cfg();
  MX_TIM_PWM_Init();
  MX_TIM_IN_Init();

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int t = arg2long_d( 1, argc, argv, UVAR_t, 10, 100000 );
  int n = arg2long_d( 1, argc, argv, UVAR_n,  1,  10000 );

  int pbase = TIM_EXA->ARR;
  tim_print_cfg( TIM_EXA );
  std_out << "# T = " << t << NL;

  for( int i=0; i<n && !break_flag; ++i ) {
    int v = (i+1) * UVAR_s;
    uint32_t pv = v * ( pbase + 1 ) / 100;
    TIM_EXA->CCR1 = pv;
    delay_ms( 100 ); // wait for steady state

    TIM_IN->CNT = 0;
    delay_ms_brk( t );
    uint32_t cnt = TIM_IN->CNT;
    std_out << i << ' ' << v << ' ' << cnt << NL;
  }
  TIM_EXA->CCR1 = 0;


  // tim_print_cfg( TIM_EXA );

  return 0;
}

int cmd_tinit( int argc, const char * const * argv )
{
  // tim_cfg();
  TIM_EXA->PSC = UVAR_p;
  TIM_EXA->ARR = UVAR_a;
  TIM_EXA->CCR1 = 0;
  delay_ms( 10 );
  tim_print_cfg( TIM_EXA );
  tim_print_cfg( TIM_IN );

  return 0;
}

int cmd_set_pwm( int argc, const char * const * argv )
{
  int v = arg2long_d( 1, argc, argv, 10, 0, 100 );
  int pbase = TIM_EXA->ARR;
  uint32_t pv = v * ( pbase + 1 ) / 100;
  TIM_EXA->CCR1 = pv;
  return 0;
}

int MX_TIM_PWM_Init()
{
  tim_pwm_h.Instance               = TIM_EXA;
  tim_pwm_h.Init.Prescaler         = UVAR_p;
  tim_pwm_h.Init.Period            = UVAR_a;
  tim_pwm_h.Init.ClockDivision     = 0;
  tim_pwm_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_pwm_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim_pwm_h ) != HAL_OK ) {
    errno = 1010;
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource( &tim_pwm_h, &sClockSourceConfig );

  TIM_OC_InitTypeDef tim_oc_cfg;
  tim_oc_cfg.OCMode       = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode   = TIM_OCFAST_DISABLE;
  tim_oc_cfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  tim_oc_cfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;

  HAL_TIM_PWM_Stop( &tim_pwm_h, TIM_CHANNEL_1 );
  tim_oc_cfg.Pulse = 0;
  if( HAL_TIM_PWM_ConfigChannel( &tim_pwm_h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    errno = 1011;
    return 0;
  }
  HAL_TIM_PWM_Start( &tim_pwm_h, TIM_CHANNEL_1 );

  return 1;
}

int MX_TIM_IN_Init()
{
  tim_cnt_h.Instance           = TIM_IN;
  tim_cnt_h.Init.Prescaler     = 0;
  tim_cnt_h.Init.CounterMode   = TIM_COUNTERMODE_UP;
  tim_cnt_h.Init.Period        = 0xFFFFFFFF;
  tim_cnt_h.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if( HAL_TIM_Base_Init( &tim_cnt_h ) != HAL_OK ) {
    errno = 1000;
    return 0;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode       = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger    = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
  sSlaveConfig.TriggerFilter   = 7;
  if( HAL_TIM_SlaveConfigSynchro( &tim_cnt_h, &sSlaveConfig ) != HAL_OK ) {
    errno = 1001;
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &tim_cnt_h, &sMasterConfig ) != HAL_OK ) {
    errno = 1002;
    return 0;
  }
  TIM_IN->CR1 |= TIM_CR1_CEN;
  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_IN ) {
    TIM_IN_EN;
    TIM_IN_PIN.cfgAF( TIM_IN_AF );
  }
}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_EXA ) {
    __HAL_RCC_TIM1_CLK_ENABLE();
    TIM_EXA_CLKEN;

    TIM_EXA_PIN1.cfgAF( TIM_EXA_GPIOAF );
    return;
  }
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM_EXA ) {
    TIM_EXA_CLKDIS;
    return;
  }
  if( tim_baseHandle->Instance == TIM_IN ) {
    TIM_IN_DIS;
  }
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

