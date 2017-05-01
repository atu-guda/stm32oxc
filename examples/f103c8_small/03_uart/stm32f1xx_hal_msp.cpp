#include <oxc_auto.h>

extern UART_HandleTypeDef uah;

/* USART1 init function */

int MX_USART1_UART_Init(void)
{
  uah.Instance          = USART1;
  uah.Init.BaudRate     = 115200;
  uah.Init.WordLength   = UART_WORDLENGTH_8B;
  uah.Init.StopBits     = UART_STOPBITS_1;
  uah.Init.Parity       = UART_PARITY_NONE;
  uah.Init.Mode         = UART_MODE_TX_RX;
  uah.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uah.Init.OverSampling = UART_OVERSAMPLING_16;
  return( HAL_UART_Init( &uah ) == HAL_OK );
}

void HAL_UART_MspInit( UART_HandleTypeDef* uartHandle )
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if( uartHandle->Instance != USART1 ) {
    return;
  }
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();

  //  PA9  ---> USART1_TX
  //  PA10 ---> USART1_RX
  GPIO_InitStruct.Pin   = GPIO_PIN_9;
  GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_MAX;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if( uartHandle->Instance!=USART1 ) {
    return;
  }
  __HAL_RCC_USART1_CLK_DISABLE();
  HAL_GPIO_DeInit( GPIOA, GPIO_PIN_9|GPIO_PIN_10 );
}

