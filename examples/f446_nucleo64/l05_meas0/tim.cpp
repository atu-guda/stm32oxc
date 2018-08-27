#include <oxc_auto.h>

#include "meas0.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim8;

static const decltype( TIM_CHANNEL_1 ) all_chs[4] =
{ TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };

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
  return 1;
}

int MX_TIM2_Init()
{
  htim2.Instance           = TIM2;
  htim2.Init.Prescaler     = 41;
  htim2.Init.CounterMode   = TIM_COUNTERMODE_UP;
  htim2.Init.Period        = 200000; // TODO: param
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if( HAL_TIM_Base_Init( &htim2 ) != HAL_OK ) {
    return 0;
  }

  if( ! tim_base_config( &htim2 ) ) {
    return 0;
  }

  if( HAL_TIM_PWM_Init( &htim2 ) != HAL_OK ) {
    return 0;
  }

  TIM_OC_InitTypeDef sConfigOC;
  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.Pulse      = 500;

  for( auto ch : all_chs ) {
    HAL_TIM_PWM_ConfigChannel( &htim2, &sConfigOC, ch );
    HAL_TIM_PWM_Start( &htim2, ch );
  }

  return 1;
}

int MX_TIM3_Init()
{
  htim3.Instance = TIM3;
  htim3.Init.Prescaler     = 0;
  htim3.Init.CounterMode   = TIM_COUNTERMODE_UP;
  htim3.Init.Period        = 0;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if( HAL_TIM_Base_Init( &htim3 ) != HAL_OK ) {
    return 0;
  }

  if( ! tim_base_config( &htim3 ) ) {
    return 0;
  }

  if( HAL_TIM_IC_Init( &htim3 ) != HAL_OK ) {
    return 0;
  }

  TIM_IC_InitTypeDef sConfigIC;
  sConfigIC.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter    = 0;
  for( auto ch : all_chs ) {
    HAL_TIM_IC_ConfigChannel( &htim3, &sConfigIC, ch );
    // HAL_TIM_XXX_Start( &htim3, ch );
  }
  return 1;
}

int MX_TIM8_Init()
{
  htim8.Instance               = TIM8;
  htim8.Init.Prescaler         = 1679;
  htim8.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim8.Init.Period            = 1000;
  htim8.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &htim8 ) != HAL_OK )  {
    return 0;
  }

  if( ! tim_base_config( &htim8 ) ) {
    return 0;
  }

  return 1;
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{
  GPIO_InitTypeDef gio;
  if( tim_baseHandle->Instance == TIM2 )  {
    __HAL_RCC_TIM2_CLK_ENABLE();
    //* A0 --> TIM2_CH1, A1 --> TIM2_CH2, B2 --> TIM2_CH4, B10 --> TIM2_CH3
    gio.Pin       = GPIO_PIN_0 | GPIO_PIN_1;
    gio.Mode      = GPIO_MODE_AF_PP;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gio.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init( GPIOA, &gio );

    gio.Pin = GPIO_PIN_2 | GPIO_PIN_10;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gio.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOB, &gio);
  } else if( tim_baseHandle->Instance == TIM3 ) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    //* A6 --> TIM3_CH1, A7 --> TIM3_CH2, B0 --> TIM3_CH3, B1 --> TIM3_CH4
    gio.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
    gio.Mode      = GPIO_MODE_AF_PP;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gio.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init( GPIOA, &gio );

    gio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init( GPIOB, &gio );

    // HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
    // HAL_NVIC_EnableIRQ(TIM3_IRQn);
  } else if( tim_baseHandle->Instance == TIM8 ) {
    __HAL_RCC_TIM8_CLK_ENABLE();
  }
}



void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM2 ) {
    __HAL_RCC_TIM2_CLK_DISABLE();
  } else if( tim_baseHandle->Instance == TIM3 )  {
    __HAL_RCC_TIM3_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_6|GPIO_PIN_7 );
    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_0|GPIO_PIN_1 );
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
  } else if( tim_baseHandle->Instance == TIM8 )  {
    __HAL_RCC_TIM8_CLK_DISABLE();
  }
}

