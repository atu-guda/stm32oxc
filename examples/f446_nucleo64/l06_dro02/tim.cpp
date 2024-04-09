#include <oxc_auto.h>

#include "dro02.h"


TIM_HandleTypeDef htim2;
volatile uint32_t tim2_catch { 0 };

void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &htim2 );
}

void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )
{
  if( htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1 )  {
    tim2_catch = TIM2->CCR1; //HAL_TIM_ReadCapturedValue( htim, TIM_CHANNEL_1 );
    TIM2->CNT = 0;
  }
}


static TIM_MasterConfigTypeDef sMasterConfig_def =
{ .MasterOutputTrigger = TIM_TRGO_RESET, .MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE };


static int tim_base_config( TIM_HandleTypeDef *th );

int tim_base_config( TIM_HandleTypeDef *th )
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( th, &sClockSourceConfig ) != HAL_OK ) {
    return 0;
  }
  if( HAL_TIMEx_MasterConfigSynchronization( th, &sMasterConfig_def ) != HAL_OK ) {
    return 0;
  }
  // UVAR('x') |= 2;
  return 1;
}


int MX_TIM2_Init()
{
  // UVAR('x') |= 1;
  htim2.Instance           = TIM2;
  htim2.Init.Prescaler         = calc_TIM_psc_for_cnt_freq( TIM2, 1000000 );
  htim2.Init.CounterMode   = TIM_COUNTERMODE_UP;
  htim2.Init.Period        = 0xFFFFFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim2 ) != HAL_OK ) {
    return 0;
  }

  if( ! tim_base_config( &htim2 ) ) {
    return 0;
  }

  TIM_IC_InitTypeDef tim_ic_cfg;
  tim_ic_cfg.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  tim_ic_cfg.ICSelection = TIM_ICSELECTION_DIRECTTI;
  tim_ic_cfg.ICPrescaler = TIM_ICPSC_DIV1;
  tim_ic_cfg.ICFilter    = 0;
  if( HAL_TIM_IC_ConfigChannel( &htim2, &tim_ic_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }

  if( HAL_TIM_IC_Start_IT( &htim2, TIM_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }
  // UVAR('x') |= 4;
  return 1;
}


void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM2 )  { // PWM input
    __HAL_RCC_TIM2_CLK_ENABLE();
    //* A0 --> TIM2_CH1
    GpioA.cfgAF_N( GPIO_PIN_0, GPIO_AF1_TIM2 );
    HAL_NVIC_SetPriority( TIM2_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( TIM2_IRQn );
    // UVAR('x') |= 8;
  }
}


void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM2 )  {
    __HAL_RCC_TIM2_CLK_DISABLE();
  }
}

