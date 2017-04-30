#include <oxc_base.h>

int init_uart( UART_HandleTypeDef *uah, int baud )
{
  uah->Instance          = UART8;
  uah->Init.BaudRate     = baud;
  uah->Init.WordLength   = UART_WORDLENGTH_8B;
  uah->Init.StopBits     = UART_STOPBITS_1;
  uah->Init.Parity       = UART_PARITY_NONE;
  uah->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uah->Init.Mode         = UART_MODE_TX_RX;
  uah->Init.OverSampling = UART_OVERSAMPLING_16;
  // advance for f3
  return( HAL_UART_Init( uah ) == HAL_OK );
}

void HAL_UART_MspInit( UART_HandleTypeDef* uah )
{
  GPIO_InitTypeDef gio;
  if( uah->Instance == UART8 ) {
    __UART8_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();

    // E1     ------> UART8_TX
    // E0     ------> UART8_RX
    gio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gio.Alternate = GPIO_AF8_UART8;
    HAL_GPIO_Init( GPIOE, &gio );

    // IRQ
    HAL_NVIC_SetPriority( UART8_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
    HAL_NVIC_EnableIRQ( UART8_IRQn );
  }
}

void HAL_UART_MspDeInit( UART_HandleTypeDef* uah )
{
  if( uah->Instance == UART8 )  {
    __UART8_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOE, GPIO_PIN_0 | GPIO_PIN_1 );
    HAL_NVIC_DisableIRQ( UART8_IRQn );
  }

}

