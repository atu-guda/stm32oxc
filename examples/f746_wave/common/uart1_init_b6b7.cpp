#include <oxc_base.h>

int init_uart( UART_HandleTypeDef *uah, int baud )
{
  uah->Instance                    = USART1;
  uah->Init.BaudRate               = 115200;
  uah->Init.WordLength             = UART_WORDLENGTH_8B;
  uah->Init.StopBits               = UART_STOPBITS_1;
  uah->Init.Parity                 = UART_PARITY_NONE;
  uah->Init.Mode                   = UART_MODE_TX_RX;
  uah->Init.HwFlowCtl              = UART_HWCONTROL_NONE;
  uah->Init.OverSampling           = UART_OVERSAMPLING_16;
  uah->Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
  uah->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  return( HAL_UART_Init( uah ) == HAL_OK );
}

void HAL_UART_MspInit( UART_HandleTypeDef* uah )
{
  GPIO_InitTypeDef gio;
  if( uah->Instance != USART1 ) {
    return;
  }

  __GPIOB_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  // B6     ------> USART1_TX
  // B7     ------> USART1_RX
  gio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  gio.Mode = GPIO_MODE_AF_PP;
  gio.Pull = GPIO_PULLUP;
  gio.Speed = GPIO_SPEED_MAX;
  gio.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init( GPIOB, &gio );

  HAL_NVIC_SetPriority( USART1_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( USART1_IRQn );
}

void HAL_UART_MspDeInit( UART_HandleTypeDef* uah )
{
  if( uah->Instance != USART1 ) {
    return;
  }
  __HAL_RCC_USART1_CLK_DISABLE();
  HAL_GPIO_DeInit( GPIOB, GPIO_PIN_6 | GPIO_PIN_7 );
  HAL_NVIC_DisableIRQ( USART1_IRQn );
}

