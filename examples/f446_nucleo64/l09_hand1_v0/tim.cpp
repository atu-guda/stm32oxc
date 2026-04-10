#include <cerrno>
#include <oxc_auto.h>

#include "main.h"

TIM_HandleTypeDef htim_pwm;

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle );



int MX_TIM_PWM_Init(void)
{
  const uint32_t psc { calc_TIM_psc_for_cnt_freq( TIM_PWM, tim_pwm_psc_freq ) };
  tim_pwm_arr = calc_TIM_arr_for_base_psc( TIM_PWM, psc, tim_pwm_freq );
  UVAR('a') = psc;
  UVAR('b') = tim_pwm_arr;

  htim_pwm.Instance               = TIM_PWM;
  htim_pwm.Init.Prescaler         = psc; // TODO: config
  htim_pwm.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim_pwm.Init.Period            = tim_pwm_arr;
  htim_pwm.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim_pwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim_pwm ) != HAL_OK ) {
    errno = 15001;
    return 0;
  }

  static const TIM_ClockConfigTypeDef sClockSourceConfig = {
    .ClockSource    = TIM_CLOCKSOURCE_INTERNAL,
    .ClockPolarity  = TIM_CLOCKPOLARITY_NONINVERTED,
    .ClockPrescaler = TIM_CLOCKPRESCALER_DIV1,
    .ClockFilter    = 0
  };
  if( HAL_TIM_ConfigClockSource( &htim_pwm, &sClockSourceConfig ) != HAL_OK ) {
    errno = 15002;
    return 0;
  }

  if( HAL_TIM_PWM_Init( &htim_pwm ) != HAL_OK ) {
    errno = 15003;
    return 0;
  }

  static const TIM_MasterConfigTypeDef sMasterConfig = {
    .MasterOutputTrigger = TIM_TRGO_RESET,
    .MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE
  };
  if( HAL_TIMEx_MasterConfigSynchronization( &htim_pwm, &sMasterConfig ) != HAL_OK ) {
    errno = 15004;
    return 0;
  }

  static const TIM_OC_InitTypeDef sConfigOC = {
    .OCMode       = TIM_OCMODE_PWM1,
    .Pulse        = 0,
    .OCPolarity   = TIM_OCPOLARITY_HIGH,
    .OCNPolarity  = TIM_OCNPOLARITY_HIGH,
    .OCFastMode   = TIM_OCFAST_DISABLE,
    .OCIdleState = 0,
    .OCNIdleState = 0,
  };
  if( HAL_TIM_PWM_ConfigChannel( &htim_pwm, &sConfigOC, TIM_PWM_CHANNEL ) != HAL_OK ) {
    errno = 15005;
    return 0;
  }

  HAL_TIM_MspPostInit( &htim_pwm );

  HAL_TIM_PWM_Start( &htim_pwm, TIM_PWM_CHANNEL );

  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM_PWM ) {
    TIM_PWM_EN;
    TIM_PWM_PIN.enableClk();
    TIM_PWM_PIN.cfgAF( TIM_PWM_AF );
  }
}

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle )
{
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM_PWM ) {
    TIM_PWM_DIS;
    TIM_PWM_PIN.cfgIn();
  }
}


