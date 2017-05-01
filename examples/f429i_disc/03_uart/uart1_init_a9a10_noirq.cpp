#include <oxc_base.h>
// not a common file: w/o irq

int init_uart( UART_HandleTypeDef *uah, int baud )
{
  uah->Instance = USART1;
  uah->Init.BaudRate     = baud;
  uah->Init.WordLength   = UART_WORDLENGTH_8B;
  uah->Init.StopBits     = UART_STOPBITS_1;
  uah->Init.Parity       = UART_PARITY_NONE;
  uah->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uah->Init.Mode         = UART_MODE_TX_RX;
  uah->Init.OverSampling = UART_OVERSAMPLING_16;
  return( HAL_UART_Init( uah ) == HAL_OK );
}

void HAL_UART_MspInit( UART_HandleTypeDef* huart )
{

  GPIO_InitTypeDef gio;
  if( huart->Instance == USART1 ) {
    __USART1_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();

    // A9     ------> USART2_TX
    // A10    ------> USART2_RX
    gio.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_MAX;
    gio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init( GPIOA, &gio );

    /* Peripheral interrupt init*/
    // wait: not now
    // HAL_NVIC_SetPriority( USART2_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
    // HAL_NVIC_EnableIRQ( USART2_IRQn );
  }

}

void HAL_UART_MspDeInit( UART_HandleTypeDef* huart )
{
  if( huart->Instance == USART1 )  {
    __USART1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_9 | GPIO_PIN_10 );
    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ( USART1_IRQn );
  }

}

