#include <cerrno>
#include <oxc_auto.h>

#include "main.h"

TIM_HandleTypeDef htim_cnt;
TIM_HandleTypeDef htim_lwm;

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle );

// TIM2 = TIM_CNT
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

  static const TIM_Encoder_InitTypeDef sConfig {
    .EncoderMode  = TIM_ENCODERMODE_TI1,
    .IC1Polarity  = TIM_ICPOLARITY_RISING,
    .IC1Selection = TIM_ICSELECTION_DIRECTTI,
    .IC1Prescaler = TIM_ICPSC_DIV1,
    .IC1Filter    = 3,
    .IC2Polarity  = TIM_ICPOLARITY_RISING,
    .IC2Selection = TIM_ICSELECTION_DIRECTTI,
    .IC2Prescaler = TIM_ICPSC_DIV1,
    .IC2Filter    = 3
  };
  if( HAL_TIM_Encoder_Init( &htim_cnt, &sConfig ) != HAL_OK ) {
    errno = 12002;
    return 0;
  }

  static const TIM_MasterConfigTypeDef sMasterConfig = {
    .MasterOutputTrigger = TIM_TRGO_RESET,
    .MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE
  };

  if( HAL_TIMEx_MasterConfigSynchronization( &htim_cnt, &sMasterConfig ) != HAL_OK ) {
    errno = 12002;
    return 0;
  }
  return 1;
}



int MX_TIM_LWM_Init(void)
{
  const uint32_t psc { calc_TIM_psc_for_cnt_freq( TIM_LWM, tim_lwm_psc_freq ) };
  const auto tim_lwm_arr = calc_TIM_arr_for_base_psc( TIM_LWM, psc, tim_lwm_freq );
  UVAR('a') = psc;
  UVAR('b') = tim_lwm_arr;

  htim_lwm.Instance               = TIM_LWM;
  htim_lwm.Init.Prescaler         = psc; // TODO: config
  htim_lwm.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim_lwm.Init.Period            = tim_lwm_arr;
  htim_lwm.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim_lwm.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim_lwm ) != HAL_OK ) {
    errno = 15001;
    return 0;
  }

  static const TIM_ClockConfigTypeDef sClockSourceConfig = {
    .ClockSource    = TIM_CLOCKSOURCE_INTERNAL,
    .ClockPolarity  = TIM_CLOCKPOLARITY_NONINVERTED,
    .ClockPrescaler = TIM_CLOCKPRESCALER_DIV1,
    .ClockFilter    = 0
  };
  if( HAL_TIM_ConfigClockSource( &htim_lwm, &sClockSourceConfig ) != HAL_OK ) {
    errno = 15002;
    return 0;
  }

  if( HAL_TIM_PWM_Init( &htim_lwm ) != HAL_OK ) {
    errno = 15003;
    return 0;
  }

  static const TIM_MasterConfigTypeDef sMasterConfig = {
    .MasterOutputTrigger = TIM_TRGO_RESET,
    .MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE
  };
  if( HAL_TIMEx_MasterConfigSynchronization( &htim_lwm, &sMasterConfig ) != HAL_OK ) {
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
  if( HAL_TIM_PWM_ConfigChannel( &htim_lwm, &sConfigOC, TIM_LWM_CHANNEL ) != HAL_OK ) {
    errno = 15005;
    return 0;
  }

  HAL_TIM_MspPostInit( &htim_lwm );

  HAL_TIM_PWM_Start( &htim_lwm, TIM_LWM_CHANNEL );

  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM_CNT ) {
    TIM_CNT_EN;
    TIM_CNT_GPIO.cfgAF_N( TIM_CNT_PINS, TIM_CNT_AF );
    TIM_CNT_GPIO.cfg_set_pull_down( TIM_CNT_PIN0NUM );
    TIM_CNT_GPIO.cfg_set_pull_down( TIM_CNT_PIN1NUM );
  }
  else if( tim_baseHandle->Instance == TIM_LWM ) {
    TIM_LWM_EN;
    TIM_LWM_GPIO.cfgAF_N( TIM_LWM_PIN, TIM_LWM_AF );
  }
}

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* timHandle )
{
}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM_CNT ) {
    TIM_CNT_DIS;
    TIM_CNT_GPIO.cfgIn_N( TIM_CNT_PINS );
  }
  else if( tim_baseHandle->Instance == TIM_LWM ) {
    TIM_LWM_DIS;
    TIM_LWM_GPIO.cfgIn_N( TIM_LWM_PIN );
  }
}


