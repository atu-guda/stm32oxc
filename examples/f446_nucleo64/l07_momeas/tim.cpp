#include <oxc_auto.h>

#include "momeas.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim5;

// TIM2
int MX_TIM_CNT_Init(void)
{
  htim2.Instance               = TIM_CNT;
  htim2.Init.Prescaler         = 0;
  htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim2.Init.Period            = 0xFFFFFFFF;
  htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init( &htim2 ) != HAL_OK ) {
    errno = 12001;
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig = {
    .ClockSource    = TIM_CLOCKSOURCE_ETRMODE2,
    .ClockPolarity  = TIM_CLOCKPOLARITY_NONINVERTED,
    .ClockPrescaler = TIM_CLOCKPRESCALER_DIV1,
    .ClockFilter    = 0
  };

  if( HAL_TIM_ConfigClockSource( &htim2, &sClockSourceConfig ) != HAL_OK ) {
    errno = 12002;
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig = {
    .MasterOutputTrigger = TIM_TRGO_RESET,
    .MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE
  };
  if( HAL_TIMEx_MasterConfigSynchronization( &htim2, &sMasterConfig ) != HAL_OK ) {
    errno = 12003;
    return 0;
  }
  return 1;
}


int MX_TIM_PWM_Init(void)
{

  htim5.Instance               = TIM_PWM;
  htim5.Init.Prescaler         = 0;
  htim5.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim5.Init.Period            = 0xFFFFFFFF;
  htim5.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if( HAL_TIM_Base_Init(&htim5) != HAL_OK ) {
    errno = 15001;
    return 0;
  }

  TIM_ClockConfigTypeDef sClockSourceConfig = {
    .ClockSource = TIM_CLOCKSOURCE_INTERNAL
  }
  if( HAL_TIM_ConfigClockSource( &htim5, &sClockSourceConfig ) != HAL_OK ) {
    errno = 15002;
    return 0;
  }

  if( HAL_TIM_PWM_Init( &htim5 ) != HAL_OK ) {
    errno = 15003;
    return 0;
  }

  TIM_MasterConfigTypeDef sMasterConfig = {
    .MasterOutputTrigger = TIM_TRGO_RESET,
    .MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE
  };
  if( HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK ) {
    errno = 15004;
    return 0;
  }

  TIM_OC_InitTypeDef sConfigOC = {;
    sConfigOC.OCMode     = TIM_OCMODE_PWM1,
    sConfigOC.Pulse      = 0,
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH,
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_2) != HAL_OK ) {
    errno = 15005;
    return 0;
  }

  HAL_TIM_MspPostInit(&htim5);

  return 1;
}

void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* tim_baseHandle )
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if( tim_baseHandle->Instance == TIM_CNT ) {

    __HAL_RCC_TIM_CNT_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = T2_ETR_COUNT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = TIM_CNT_AF;
    HAL_GPIO_Init(T2_ETR_COUNT_GPIO_Port, &GPIO_InitStruct);
  }
  else if( tim_baseHandle->Instance == TIM_PWM ) {
    __HAL_RCC_TIM_PWM_CLK_ENABLE();
  }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if( timHandle->Instance==TIM_PWM ) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM5 GPIO Configuration PA1     ------> TIM5_CH2 */
    GPIO_InitStruct.Pin = T5x2_PWM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = TIM_PWM_AF;
    HAL_GPIO_Init(T5x2_PWM_GPIO_Port, &GPIO_InitStruct);
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{
  if( tim_baseHandle->Instance == TIM_CNT ) {
    TIM_CNT_DIS;
    // HAL_GPIO_DeInit(T2_ETR_COUNT_GPIO_Port, T2_ETR_COUNT_Pin);
  }
  else if( tim_baseHandle->Instance == TIM_PWM ) {
    TIM_PWM_DIS;
  }
}


