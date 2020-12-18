#include <cerrno>

#include <oxc_auto.h>

using namespace std;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

int MX_TIM1_Init();
int MX_TIM2_Init();
int MX_TIM4_Init();
int MX_TIM5_Init();

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* htim );


int MX_TIM1_Init()
{
  htim1.Instance               = TIM1;
  htim1.Init.Prescaler         = 0;
  htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim1.Init.Period            = 65535;
  htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_PWM_Init( &htim1 ) != HAL_OK ) {
    errno = 7711; return 1;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &htim1, &sMasterConfig ) != HAL_OK ) {
    errno = 7712; return 1;
  }

  TIM_OC_InitTypeDef sConfigOC;
  sConfigOC.OCMode       = TIM_OCMODE_PWM1;
  sConfigOC.Pulse        = 0;
  sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if( HAL_TIM_PWM_ConfigChannel( &htim1, &sConfigOC, TIM_CHANNEL_1 ) != HAL_OK ) {
    errno = 7713; return 1;
  }

  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig ;
  sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime         = 0;
  sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
  if( HAL_TIMEx_ConfigBreakDeadTime( &htim1, &sBreakDeadTimeConfig ) != HAL_OK ) {
    errno = 7714; return 1;
  }
  HAL_TIM_MspPostInit( &htim1 );
  return 0;
}

int MX_TIM2_Init()
{
  htim2.Instance               = TIM2;
  htim2.Init.Prescaler         = 0;
  htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim2.Init.Period            = 0xFFFFFFFF;
  htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim2 ) != HAL_OK ) {
    errno = 7721; return 1;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig ;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( &htim2, &sClockSourceConfig ) != HAL_OK ) {
    errno = 7722; return 1;
  }

  if( HAL_TIM_IC_Init( &htim2 ) != HAL_OK ) {
    errno = 7723; return 1;
  }

  TIM_MasterConfigTypeDef sMasterConfig ;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &htim2, &sMasterConfig ) != HAL_OK ) {
    errno = 7724; return 1;
  }

  TIM_IC_InitTypeDef sConfigIC ;
  sConfigIC.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter    = 0;
  if( HAL_TIM_IC_ConfigChannel( &htim2, &sConfigIC, TIM_CHANNEL_3 ) != HAL_OK ) {
    errno = 7725; return 1;
  }

  if( HAL_TIM_IC_ConfigChannel( &htim2, &sConfigIC, TIM_CHANNEL_4 ) != HAL_OK ) {
    errno = 7726; return 1;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig ;
  sSlaveConfig.SlaveMode        = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger     = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
  sSlaveConfig.TriggerFilter    = 0;
  if( HAL_TIM_SlaveConfigSynchro( &htim2, &sSlaveConfig ) != HAL_OK ) {
    errno = 7727; return 1;
  }

  if( HAL_TIM_IC_ConfigChannel( &htim2, &sConfigIC, TIM_CHANNEL_1 ) != HAL_OK ) {
    errno = 7728; return 1;
  }

  sConfigIC.ICPolarity  = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if( HAL_TIM_IC_ConfigChannel( &htim2, &sConfigIC, TIM_CHANNEL_2 ) != HAL_OK ) {
    errno = 7729; return 1;
  }

  return 0;
}

int MX_TIM4_Init()
{
  htim4.Instance               = TIM4;
  htim4.Init.Prescaler         = 0;
  htim4.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim4.Init.Period            = 65535;
  htim4.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_IC_Init( &htim4 ) != HAL_OK ) {
    errno = 7741; return 1;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig ;
  sSlaveConfig.SlaveMode        = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger     = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
  sSlaveConfig.TriggerFilter    = 0;
  if( HAL_TIM_SlaveConfigSynchro( &htim4, &sSlaveConfig ) != HAL_OK ) {
    errno = 7742; return 1;
  }

  TIM_IC_InitTypeDef sConfigIC ;
  sConfigIC.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter    = 0;
  if( HAL_TIM_IC_ConfigChannel( &htim4, &sConfigIC, TIM_CHANNEL_1 ) != HAL_OK ) {
    errno = 7743; return 1;
  }

  sConfigIC.ICPolarity  = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if( HAL_TIM_IC_ConfigChannel( &htim4, &sConfigIC, TIM_CHANNEL_2 ) != HAL_OK ) {
    errno = 7744; return 1;
  }

  TIM_MasterConfigTypeDef sMasterConfig ;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &htim4, &sMasterConfig ) != HAL_OK ) {
    errno = 7745; return 1;
  }

  return 0;
}

int MX_TIM5_Init()
{
  uint32_t arr_t5 = calc_TIM_arr_for_base_psc( TIM5, 0, 1 ); // 1 Hz init
  htim5.Instance               = TIM5;
  htim5.Init.Prescaler         = 0;
  htim5.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim5.Init.Period            = arr_t5;
  htim5.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_PWM_Init( &htim5 ) != HAL_OK ) {
    errno = 7751; return 1;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig ;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( &htim5, &sClockSourceConfig ) != HAL_OK ) {
    errno = 7752; return 1;
  }

  TIM_MasterConfigTypeDef sMasterConfig ;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &htim5, &sMasterConfig ) != HAL_OK ) {
    errno = 7753; return 1;
  }

  TIM_OC_InitTypeDef sConfigOC ;
  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.Pulse      = arr_t5 / 10;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if( HAL_TIM_PWM_ConfigChannel( &htim5, &sConfigOC, TIM_CHANNEL_1 ) != HAL_OK ) {
    errno = 7755; return 1;
  }

  sConfigOC.Pulse      = arr_t5 / 5;
  if( HAL_TIM_PWM_ConfigChannel( &htim5, &sConfigOC, TIM_CHANNEL_2 ) != HAL_OK ) {
    errno = 7756; return 1;
  }

  sConfigOC.Pulse      = arr_t5 / 3;
  if( HAL_TIM_PWM_ConfigChannel( &htim5, &sConfigOC, TIM_CHANNEL_3 ) != HAL_OK ) {
    errno = 7757; return 1;
  }

  sConfigOC.Pulse      = arr_t5 / 2;
  if( HAL_TIM_PWM_ConfigChannel( &htim5, &sConfigOC, TIM_CHANNEL_4 ) != HAL_OK ) {
    errno = 7758; return 1;
  }
  // HAL_TIM_MspPostInit( &htim5 );
  HAL_TIM_PWM_Start( &htim5, TIM_CHANNEL_1 );
  HAL_TIM_PWM_Start( &htim5, TIM_CHANNEL_2 );
  HAL_TIM_PWM_Start( &htim5, TIM_CHANNEL_3 );
  HAL_TIM_PWM_Start( &htim5, TIM_CHANNEL_4 );

  return 0;
}


void TIM2_IRQHandler()
{
  HAL_TIM_IRQHandler( &htim2 );
}

void TIM4_IRQHandler()
{
  HAL_TIM_IRQHandler( &htim4 );
}


void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim_pwm )
{
  if( htim_pwm->Instance == TIM1 ) {
    __HAL_RCC_TIM1_CLK_ENABLE();
  } else if ( htim_pwm->Instance == TIM5 ) {
    __HAL_RCC_TIM5_CLK_ENABLE();
    // TIM5 GPIO Configuration: A0-A3 --> T5_CH1-4
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GpioA.cfgAF_N( GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_AF2_TIM5 );
  }
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* htim_base )
{
  GPIO_InitTypeDef GPIO_InitStruct ;

  if( htim_base->Instance == TIM2 ) {
    __HAL_RCC_TIM2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PB10     ------> TIM2_CH3
    PB11     ------> TIM2_CH4
    PA15     ------> TIM2_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    HAL_NVIC_SetPriority( TIM2_IRQn, 12, 0 );
    HAL_NVIC_EnableIRQ( TIM2_IRQn );
  }

}

void HAL_TIM_IC_MspInit( TIM_HandleTypeDef* htim_ic )
{
  GPIO_InitTypeDef GPIO_InitStruct ;
  if( htim_ic->Instance == TIM4 ) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**TIM4 GPIO Configuration PD12     ------> TIM4_CH1 */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );

    HAL_NVIC_SetPriority( TIM4_IRQn, 12, 0 );
    HAL_NVIC_EnableIRQ( TIM4_IRQn );
  }

}

void HAL_TIM_MspPostInit( TIM_HandleTypeDef* htim )
{
  GPIO_InitTypeDef GPIO_InitStruct ;

  if( htim->Instance == TIM1 ) {
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PE8     ------> TIM1_CH1N
    PE9     ------> TIM1_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
  }

}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim_pwm )
{
  if( htim_pwm->Instance == TIM1 ) {
    __HAL_RCC_TIM1_CLK_DISABLE();
  }

}

void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* htim_base )
{
  if( htim_base->Instance==TIM2 ) {
    __HAL_RCC_TIM2_CLK_DISABLE();

    // HAL_GPIO_DeInit( GPIOB, GPIO_PIN_10|GPIO_PIN_11 );
    // HAL_GPIO_DeInit( GPIOA, GPIO_PIN_15 );

    HAL_NVIC_DisableIRQ( TIM2_IRQn );
  } else if( htim_base->Instance == TIM5 ) {
    __HAL_RCC_TIM5_CLK_DISABLE();
  }

}

void HAL_TIM_IC_MspDeInit( TIM_HandleTypeDef* htim_ic )
{
  if( htim_ic->Instance == TIM4 ) {
    __HAL_RCC_TIM4_CLK_DISABLE();
    /**TIM4 GPIO Configuration PD12     ------> TIM4_CH1 */
    HAL_GPIO_DeInit( GPIOD, GPIO_PIN_12 );
    HAL_NVIC_DisableIRQ( TIM4_IRQn );
  }

}

