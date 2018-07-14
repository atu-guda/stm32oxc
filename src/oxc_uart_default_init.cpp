#include <string.h>
#include <oxc_base.h>

// if You need more then one UART,  of requre special actions, do not use
// this file.


int init_uart( UART_HandleTypeDef *uah, int baud )
{
  if( !uah ) {
    return 0;
  }
  memset( uah, 0, sizeof(*uah) ); // to set to 0 all advanced featires, if any
  uah->Instance          = BOARD_UART_DEFAULT;
  uah->Init.BaudRate     = baud;
  uah->Init.WordLength   = UART_WORDLENGTH_8B;
  uah->Init.StopBits     = UART_STOPBITS_1;
  uah->Init.Parity       = UART_PARITY_NONE;
  uah->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uah->Init.Mode         = UART_MODE_TX_RX;
  uah->Init.OverSampling = UART_OVERSAMPLING_16;
  return( HAL_UART_Init( uah ) == HAL_OK );
}


void HAL_UART_MspInit( UART_HandleTypeDef* uah )
{
  GPIO_InitTypeDef gio;
  if( uah->Instance == BOARD_UART_DEFAULT ) {
    BOARD_UART_DEFAULT_ENABLE;

    #if ! defined (STM32F1)
    gio.Pin       = BOARD_UART_DEFAULT_GPIO_PINS;
    gio.Mode      = GPIO_MODE_AF_PP;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_MAX;
    gio.Alternate = BOARD_UART_DEFAULT_GPIO_AF;
    HAL_GPIO_Init( BOARD_UART_DEFAULT_GPIO, &gio );
    #else
    gio.Pin       = BOARD_UART_DEFAULT_GPIO_TX;
    gio.Mode      = GPIO_MODE_AF_PP;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_MAX;
    HAL_GPIO_Init( BOARD_UART_DEFAULT_GPIO, &gio );
    //
    gio.Pin       = BOARD_UART_DEFAULT_GPIO_RX;
    gio.Mode      = GPIO_MODE_INPUT;
    gio.Pull      = GPIO_NOPULL;
    HAL_GPIO_Init( BOARD_UART_DEFAULT_GPIO, &gio );
    #endif

    #ifndef UART_DEFALT_NO_IRQ
    HAL_NVIC_SetPriority( BOARD_UART_DEFAULT_IRQ, OXC_DEFAULT_UART_PRTY, 0 );
    HAL_NVIC_EnableIRQ( BOARD_UART_DEFAULT_IRQ );
    #endif
  }
}

void HAL_UART_MspDeInit( UART_HandleTypeDef* uah )
{
  if( uah->Instance == BOARD_UART_DEFAULT ) {
    BOARD_UART_DEFAULT_DISABLE;
    HAL_GPIO_DeInit( BOARD_UART_DEFAULT_GPIO, BOARD_UART_DEFAULT_GPIO_PINS );
    HAL_NVIC_DisableIRQ( BOARD_UART_DEFAULT_IRQ );
  }
}

