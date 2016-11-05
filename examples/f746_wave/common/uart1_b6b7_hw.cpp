#include <oxc_base.h>

extern UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if( HAL_UART_Init( &huart1 ) != HAL_OK )  {
    Error_Handler( 2 );
  }
}

void HAL_UART_MspInit( UART_HandleTypeDef* uartHandle )
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if( uartHandle->Instance != USART1 ) {
    return;
  }

  __GPIOB_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  /** USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX  */
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

  HAL_NVIC_SetPriority( USART1_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( USART1_IRQn );
}

void HAL_UART_MspDeInit( UART_HandleTypeDef* uartHandle )
{
  if( uartHandle->Instance != USART1 ) {
    return;
  }
  __HAL_RCC_USART1_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);
  HAL_NVIC_DisableIRQ( USART1_IRQn );
}

