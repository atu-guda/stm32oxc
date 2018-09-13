#include <oxc_auto.h>

#include "meas0.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
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

int MX_TIM1_Init()
{
  htim1.Instance               = TIM1;
  htim1.Init.Prescaler         = 0;
  htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim1.Init.Period            = 0xFFFF;
  htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if( HAL_TIM_IC_Init( &htim1 ) != HAL_OK ) {
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( &htim1, &sClockSourceConfig ) != HAL_OK ) {
    return 0;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode     = TIM_SLAVEMODE_DISABLE;
  sSlaveConfig.InputTrigger  = TIM_TS_TI1F_ED;
  sSlaveConfig.TriggerFilter = 0;
  if( HAL_TIM_SlaveConfigSynchronization( &htim1, &sSlaveConfig ) != HAL_OK ) {
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &htim1, &sMasterConfig ) != HAL_OK ) {
    return 0;
  }

  return 1;
}

int MX_TIM2_Init()
{
  htim2.Instance           = TIM2;
  htim2.Init.Prescaler     = 0;
  htim2.Init.CounterMode   = TIM_COUNTERMODE_UP;
  htim2.Init.Period        = 2000000; // TODO: param
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if( HAL_TIM_PWM_Init( &htim2 ) != HAL_OK ) {
    return 0;
  }

  if( ! tim_base_config( &htim2 ) ) {
    return 0;
  }


  TIM_OC_InitTypeDef sConfigOC;
  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.Pulse      = 500; // TODO: set to 0 in production

  for( auto ch : all_chs ) {
    HAL_TIM_PWM_ConfigChannel( &htim2, &sConfigOC, ch );
    HAL_TIM_PWM_Start( &htim2, ch );
  }

  return 1;
}

int MX_TIM3_Init()
{
  htim3.Instance = TIM3;
  htim3.Init.Prescaler     = calc_TIM_psc_for_cnt_freq( TIM3, 200000 ); // 0-20000 for 0.1s
  htim3.Init.CounterMode   = TIM_COUNTERMODE_UP;
  htim3.Init.Period        = 0xFFFF;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if( HAL_TIM_IC_Init( &htim3 ) != HAL_OK ) {
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( &htim3, &sClockSourceConfig ) != HAL_OK )  {
    return 0;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode        = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger     = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
  sSlaveConfig.TriggerFilter    = 0;
  if( HAL_TIM_SlaveConfigSynchronization( &htim3, &sSlaveConfig ) != HAL_OK ) {
    return 0;
  }

  TIM_IC_InitTypeDef sConfig;
  sConfig.ICPrescaler = TIM_ICPSC_DIV1;
  sConfig.ICFilter    = 0;
  sConfig.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
  if( HAL_TIM_IC_ConfigChannel( &htim3, &sConfig, TIM_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }

  sConfig.ICPolarity  = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfig.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if( HAL_TIM_IC_ConfigChannel( &htim3, &sConfig, TIM_CHANNEL_2 ) != HAL_OK ) {
    return 0;
  }



  if( HAL_TIM_IC_Start_IT( &htim3, TIM_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }
  if( HAL_TIM_IC_Start_IT( &htim3, TIM_CHANNEL_2 ) != HAL_OK ) {
    return 0;
  }

  return 1;
}

int MX_TIM5_Init()
{
  htim5.Instance           = TIM5;
  htim5.Init.Prescaler     = 0;
  htim5.Init.CounterMode   = TIM_COUNTERMODE_UP;
  htim5.Init.Period        = 0xFFFFFFFF;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if( HAL_TIM_IC_Init( &htim5 ) != HAL_OK ) {
    return 0;
  }

  TIM_IC_InitTypeDef sConfig;
  sConfig.ICPrescaler = TIM_ICPSC_DIV1;
  sConfig.ICFilter    = 0;
  sConfig.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
  if( HAL_TIM_IC_ConfigChannel( &htim5, &sConfig, TIM_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }

  sConfig.ICPolarity  = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfig.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if( HAL_TIM_IC_ConfigChannel( &htim5, &sConfig, TIM_CHANNEL_2 ) != HAL_OK ) {
    return 0;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode        = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger     = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
  sSlaveConfig.TriggerFilter    = 0;
  if( HAL_TIM_SlaveConfigSynchronization( &htim5, &sSlaveConfig ) != HAL_OK ) {
    return 0;
  }



  // TIM_MasterConfigTypeDef sMasterConfig;
  // sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  // sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  // if( HAL_TIMEx_MasterConfigSynchronization( &htim5, &sMasterConfig ) != HAL_OK ) {
  //   return 0;
  // }

  if( HAL_TIM_IC_Start_IT( &htim5, TIM_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }
  if( HAL_TIM_IC_Start_IT( &htim5, TIM_CHANNEL_2 ) != HAL_OK ) {
    return 0;
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

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{

  if( tim_baseHandle->Instance == TIM8 ) {
    dbg_val2 |= 0x20;
    __HAL_RCC_TIM8_CLK_ENABLE();
  }
}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  GPIO_InitTypeDef gio;
  gio.Mode  = GPIO_MODE_AF_PP;
  gio.Pull  = GPIO_NOPULL;
  gio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  if( tim_baseHandle->Instance == TIM2 )  { // PWM output
    __HAL_RCC_TIM2_CLK_ENABLE();
    //* A15 --> TIM2_CH1, A1 --> TIM2_CH2, B2 --> TIM2_CH4, B10 --> TIM2_CH3
    gio.Pin       = GPIO_PIN_1 | GPIO_PIN_15;
    gio.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init( GPIOA, &gio );

    gio.Pin       = GPIO_PIN_2 | GPIO_PIN_10;
    gio.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init( GPIOB, &gio);
  }
}


void HAL_TIM_IC_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  GPIO_InitTypeDef gio;
  gio.Mode  = GPIO_MODE_AF_PP;
  gio.Pull  = GPIO_NOPULL;
  gio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  if( tim_baseHandle->Instance == TIM1 )  { // A8: Ch1,Ch2 pwm in
    __HAL_RCC_TIM1_CLK_ENABLE();
    gio.Pin       = GPIO_PIN_8;
    gio.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init( GPIOA, &gio );
  } else if( tim_baseHandle->Instance == TIM3 ) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    //* A6 --> TIM3_CH1,CH2 pwm
    gio.Pin       = GPIO_PIN_6;
    gio.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init( GPIOA, &gio );

    HAL_NVIC_SetPriority( TIM3_IRQn, 3, 0 );
    HAL_NVIC_EnableIRQ( TIM3_IRQn );
  } else if( tim_baseHandle->Instance == TIM4 ) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    gio.Pin       = GPIO_PIN_6;
    gio.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init( GPIOB, &gio );

  } else if( tim_baseHandle->Instance == TIM5 ) {
    __HAL_RCC_TIM5_CLK_ENABLE();
    gio.Pin       = GPIO_PIN_0;
    gio.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init( GPIOA, &gio );
    HAL_NVIC_SetPriority( TIM5_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( TIM5_IRQn );
  }
}



void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM1 ) {
    __HAL_RCC_TIM1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_8 );
  } else if( tim_baseHandle->Instance == TIM2 )  {
    __HAL_RCC_TIM2_CLK_DISABLE();
  } else if( tim_baseHandle->Instance == TIM3 )  {
    __HAL_RCC_TIM3_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_6 );
    HAL_NVIC_DisableIRQ( TIM3_IRQn );
  } else if( tim_baseHandle->Instance == TIM4 )  {
    __HAL_RCC_TIM4_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_6 );
  } else if( tim_baseHandle->Instance == TIM5 )  {
    __HAL_RCC_TIM5_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_0 );
    HAL_NVIC_DisableIRQ( TIM5_IRQn );
  } else if( tim_baseHandle->Instance == TIM8 )  {
    __HAL_RCC_TIM8_CLK_DISABLE();
  }
}

