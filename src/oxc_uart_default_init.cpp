#include <string.h>
#include <oxc_base.h>
#include <oxc_gpio.h>

// if You need more then one UART,  of require special actions, do not use
// this file.


int init_uart( UART_HandleTypeDef *uah, int baud )
{
  if( !uah ) {
    return 0;
  }
  memset( uah, 0, sizeof(*uah) ); // to set to 0 all advanced features, if any
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

#ifdef HAL_UART_USERINIT_FUN
void  HAL_UART_USERINIT_FUN( UART_HandleTypeDef* uartHandle );
#endif

void HAL_UART_MspInit( UART_HandleTypeDef* uah )
{
  if( uah->Instance == BOARD_UART_DEFAULT ) {
    BOARD_UART_DEFAULT_ENABLE;

    BOARD_UART_DEFAULT_TX.enableClk();
    BOARD_UART_DEFAULT_RX.enableClk();
    BOARD_UART_DEFAULT_TX.cfgAF( BOARD_UART_DEFAULT_GPIO_AF );
    #if ! defined (STM32F1)
    BOARD_UART_DEFAULT_RX.cfgAF( BOARD_UART_DEFAULT_GPIO_AF );
    #else
    BOARD_UART_DEFAULT_RX.cfgIn();
    #endif

    #ifndef UART_DEFALT_NO_IRQ
    HAL_NVIC_SetPriority( BOARD_UART_DEFAULT_IRQ, OXC_DEFAULT_UART_PRTY, 0 );
    HAL_NVIC_EnableIRQ( BOARD_UART_DEFAULT_IRQ );
    #endif
  }

  #ifdef HAL_UART_USERINIT_FUN
    HAL_UART_USERINIT_FUN( uah );
  #endif
}

void HAL_UART_MspDeInit( UART_HandleTypeDef* uah )
{
  if( uah->Instance == BOARD_UART_DEFAULT ) {
    BOARD_UART_DEFAULT_DISABLE;
    BOARD_UART_DEFAULT_TX.cfgIn();
    BOARD_UART_DEFAULT_RX.cfgIn();
    HAL_NVIC_DisableIRQ( BOARD_UART_DEFAULT_IRQ );
  }
}

