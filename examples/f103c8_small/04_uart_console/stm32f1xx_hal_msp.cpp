#include <oxc_auto.h>

extern UART_HandleTypeDef uah;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{
  uah.Instance          = USART1;
  uah.Init.BaudRate     = 115200;
  uah.Init.WordLength   = UART_WORDLENGTH_8B;
  uah.Init.StopBits     = UART_STOPBITS_1;
  uah.Init.Parity       = UART_PARITY_NONE;
  uah.Init.Mode         = UART_MODE_TX_RX;
  uah.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uah.Init.OverSampling = UART_OVERSAMPLING_16;
  if( HAL_UART_Init( &uah ) != HAL_OK )  {
    Error_Handler( 1 );
  }
}

void HAL_UART_MspInit( UART_HandleTypeDef* uartHandle )
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if( uartHandle->Instance != USART1 ) {
    return;
  }
  __HAL_RCC_USART1_CLK_ENABLE();

  //  PA9  ---> USART1_TX
  //  PA10 ---> USART1_RX
  GPIO_InitStruct.Pin   = GPIO_PIN_9;
  GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Peripheral interrupt init*/
  HAL_NVIC_SetPriority( USART1_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( USART1_IRQn );
}

void HAL_UART_MspDeInit( UART_HandleTypeDef* uartHandle )
{
  if( uartHandle->Instance!=USART1 ) {
    return;
  }
  HAL_NVIC_DisableIRQ( USART1_IRQn );
  __HAL_RCC_USART1_CLK_DISABLE();
  HAL_GPIO_DeInit( GPIOA, GPIO_PIN_9|GPIO_PIN_10 );
}

