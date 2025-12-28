#include <errno.h>

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

static int MX_TIM_MeasureFreq_Init(  TIM_HandleTypeDef *th, uint32_t presc );

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
  if( HAL_TIM_Base_Init( &htim1 ) != HAL_OK ) {
    errno = 10010;
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( &htim1, &sClockSourceConfig ) != HAL_OK ) {
    errno = 10011;
    return 0;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode       = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger    = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
  sSlaveConfig.TriggerFilter   = 2;
  if( HAL_TIM_SlaveConfigSynchronization( &htim1, &sSlaveConfig ) != HAL_OK ) {
    errno = 10012;
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &htim1, &sMasterConfig ) != HAL_OK ) {
    errno = 10013;
    return 0;
  }

  if( HAL_TIM_Base_Start( &htim1 ) != HAL_OK ) {
    errno = 10014;
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
  // uint32_t presc = calc_TIM_psc_for_cnt_freq( TIM3, 200000 ); // 0-20000 for 0.1s
  uint32_t presc = calc_TIM_psc_for_cnt_freq( TIM3, 200000 );
  return MX_TIM_MeasureFreq_Init( &htim3, presc );
}

int MX_TIM4_Init()
{
  htim4.Instance               = TIM4;
  htim4.Init.Prescaler         = 0;
  htim4.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim4.Init.Period            = 0xFFFF;
  htim4.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.RepetitionCounter = 0;
  if( HAL_TIM_Base_Init( &htim4 ) != HAL_OK ) {
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig;
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if( HAL_TIM_ConfigClockSource( &htim4, &sClockSourceConfig ) != HAL_OK ) {
    return 0;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode       = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger    = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
  sSlaveConfig.TriggerFilter   = 2;
  if( HAL_TIM_SlaveConfigSynchronization( &htim4, &sSlaveConfig ) != HAL_OK ) {
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( &htim4, &sMasterConfig ) != HAL_OK ) {
    return 0;
  }

  if( HAL_TIM_Base_Start( &htim4 ) != HAL_OK ) {
    return 0;
  }

  return 1;
}

int MX_TIM_MeasureFreq_Init(  TIM_HandleTypeDef *th, uint32_t presc )
{
  if( !th ) {
    return 0;
  }
  // th->Instance           = TIMn;
  th->Init.Prescaler     = presc;
  th->Init.CounterMode   = TIM_COUNTERMODE_UP;
  th->Init.Period        = 0xFFFFFFFF;
  th->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if( HAL_TIM_IC_Init( th ) != HAL_OK ) {
    return 0;
  }

  TIM_IC_InitTypeDef sConfig;
  sConfig.ICPrescaler = TIM_ICPSC_DIV1;
  sConfig.ICFilter    = 0;
  sConfig.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
  if( HAL_TIM_IC_ConfigChannel( th, &sConfig, TIM_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }

  sConfig.ICPolarity  = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfig.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if( HAL_TIM_IC_ConfigChannel( th, &sConfig, TIM_CHANNEL_2 ) != HAL_OK ) {
    return 0;
  }

  TIM_SlaveConfigTypeDef sSlaveConfig;
  sSlaveConfig.SlaveMode        = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger     = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
  sSlaveConfig.TriggerFilter    = 4;
  if( HAL_TIM_SlaveConfigSynchronization( th, &sSlaveConfig ) != HAL_OK ) {
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig;
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if( HAL_TIMEx_MasterConfigSynchronization( th, &sMasterConfig ) != HAL_OK ) {
    return 0;
  }

  if( HAL_TIM_IC_Start_IT( th, TIM_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }
  if( HAL_TIM_IC_Start_IT( th, TIM_CHANNEL_2 ) != HAL_OK ) {
    return 0;
  }

  return 1;
}



int MX_TIM5_Init()
{
  htim5.Instance = TIM5;
  return MX_TIM_MeasureFreq_Init( &htim5, 0 );
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
  if( tim_baseHandle->Instance == TIM1 )  { // A8: Ch1 : ETR1
    __HAL_RCC_TIM1_CLK_ENABLE();
    PA8.cfgAF( GPIO_AF1_TIM1 );
  } else if( tim_baseHandle->Instance == TIM4 ) { // A6: Ch1: ETR1
    __HAL_RCC_TIM4_CLK_ENABLE();
    PB6.cfgAF( GPIO_AF2_TIM4 );
  } else if( tim_baseHandle->Instance == TIM8 ) {
    __HAL_RCC_TIM8_CLK_ENABLE();
  }
}

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM2 )  { // PWM output
    __HAL_RCC_TIM2_CLK_ENABLE();
    //* A15 --> TIM2_CH1, A1 --> TIM2_CH2, B2 --> TIM2_CH4, B10 --> TIM2_CH3
    PA1.cfgAF(   GPIO_AF1_TIM2 );
    PA15.cfgAF(  GPIO_AF1_TIM2 );
    PB2.cfgAF(   GPIO_AF1_TIM2 );
    PB10.cfgAF(  GPIO_AF1_TIM2 );
  }
}


void HAL_TIM_IC_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM3 ) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    //* A6 --> TIM3_CH1,CH2 pwm
    PA6.cfgAF( GPIO_AF2_TIM3 );

    HAL_NVIC_SetPriority( TIM3_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( TIM3_IRQn );

  } else if( tim_baseHandle->Instance == TIM5 ) {
    __HAL_RCC_TIM5_CLK_ENABLE();
    PA0.cfgAF( GPIO_AF2_TIM5 );
    HAL_NVIC_SetPriority( TIM5_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( TIM5_IRQn );
  }
}



void HAL_TIM_Base_MspDeInit( TIM_HandleTypeDef* tim_baseHandle )
{
  if( tim_baseHandle->Instance == TIM1 ) {
    __HAL_RCC_TIM1_CLK_DISABLE();
  } else if( tim_baseHandle->Instance == TIM2 )  {
    __HAL_RCC_TIM2_CLK_DISABLE();
  } else if( tim_baseHandle->Instance == TIM3 )  {
    __HAL_RCC_TIM3_CLK_DISABLE();
    HAL_NVIC_DisableIRQ( TIM3_IRQn );
  } else if( tim_baseHandle->Instance == TIM4 )  {
    __HAL_RCC_TIM4_CLK_DISABLE();
  } else if( tim_baseHandle->Instance == TIM5 )  {
    __HAL_RCC_TIM5_CLK_DISABLE();
    HAL_NVIC_DisableIRQ( TIM5_IRQn );
  } else if( tim_baseHandle->Instance == TIM8 )  {
    __HAL_RCC_TIM8_CLK_DISABLE();
  }
}

