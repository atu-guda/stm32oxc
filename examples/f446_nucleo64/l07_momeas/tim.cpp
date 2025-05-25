#include <cerrno>
#include <oxc_auto.h>

#include "momeas.h"

TIM_HandleTypeDef htim_cnt;
TIM_HandleTypeDef htim_pwm;

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle );

// TIM2
int MX_TIM_CNT_Init(void)
{
  htim_cnt.Instance               = TIM_CNT;
  htim_cnt.Init.Prescaler         = 0;
  htim_cnt.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim_cnt.Init.Period            = 0xFFFFFFFF;
  htim_cnt.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim_cnt.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim_cnt ) != HAL_OK ) {
    errno = 12001;
    return 0;
  }

  static const TIM_ClockConfigTypeDef sClockSourceConfig = {
    .ClockSource    = TIM_CLOCKSOURCE_ETRMODE2,
    .ClockPolarity  = TIM_CLOCKPOLARITY_NONINVERTED,
    .ClockPrescaler = TIM_CLOCKPRESCALER_DIV1,
    .ClockFilter    = 0
  };

  if( HAL_TIM_ConfigClockSource( &htim_cnt, &sClockSourceConfig ) != HAL_OK ) {
    errno = 12002;
    return 0;
  }

  static const TIM_MasterConfigTypeDef sMasterConfig = {
    .MasterOutputTrigger = TIM_TRGO_RESET,
    .MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE
  };
  if( HAL_TIMEx_MasterConfigSynchronization( &htim_cnt, &sMasterConfig ) != HAL_OK ) {
    errno = 12003;
    return 0;
  }
  return 1;
}


int MX_TIM_PWM_Init(void)
{
  htim_pwm.Instance               = TIM_PWM;
  htim_pwm.Init.Prescaler         = 0;
  htim_pwm.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim_pwm.Init.Period            = 0xFFFFFFFF;
  htim_pwm.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim_pwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init(&htim_pwm) != HAL_OK ) {
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
  if( HAL_TIMEx_MasterConfigSynchronization(&htim_pwm, &sMasterConfig) != HAL_OK ) {
    errno = 15004;
    return 0;
  }

  static const TIM_OC_InitTypeDef sConfigOC = {
    .OCMode      = TIM_OCMODE_PWM1,
    .Pulse       = 0,
    .OCPolarity  = TIM_OCPOLARITY_HIGH,
    .OCNPolarity = TIM_OCNPOLARITY_HIGH,
    .OCFastMode  = TIM_OCFAST_DISABLE,
    .OCIdleState = 0,
    .OCNIdleState = 0,
  };
  if( HAL_TIM_PWM_ConfigChannel( &htim_pwm, &sConfigOC, TIM_PWM_CHANNEL ) != HAL_OK ) {
    errno = 15005;
    return 0;
  }

  HAL_TIM_MspPostInit( &htim_pwm );

  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM_CNT ) {
    TIM_CNT_EN;
    TIM_CNT_GPIO.cfgAF_N( TIM_CNT_PIN, TIM_CNT_AF );
  }
  else if( tim_baseHandle->Instance == TIM_PWM ) {
    TIM_PWM_EN;
    TIM_PWM_GPIO.cfgAF_N( TIM_PWM_PIN, TIM_PWM_AF );
  }
}

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle )
{
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM_CNT ) {
    TIM_CNT_DIS;
    TIM_CNT_GPIO.cfgIn_N( TIM_CNT_PIN );
  }
  else if( tim_baseHandle->Instance == TIM_PWM ) {
    TIM_PWM_DIS;
    TIM_PWM_GPIO.cfgIn_N( TIM_PWM_PIN );
  }
}


