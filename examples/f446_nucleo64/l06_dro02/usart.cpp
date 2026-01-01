#include <oxc_auto.h>
 #include <stm32f4xx_hal_usart.h>

#include "dro02.h"

UART_HandleTypeDef huart1;

int  MX_FC_UART_Init()
{
  huart1.Instance          = UART_FC;
  huart1.Init.BaudRate     = 115200;
  huart1.Init.WordLength   = UART_WORDLENGTH_8B;
  huart1.Init.StopBits     = UART_STOPBITS_1;
  huart1.Init.Parity       = UART_PARITY_NONE;
  huart1.Init.Mode         = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if( HAL_UART_Init( &huart1 ) != HAL_OK ) {
    return 0;
  }

  UVAR_x |= 1;
  return 1;
}

void HAL_UART_UserInit( UART_HandleTypeDef* uartHandle )
{
  if( uartHandle->Instance != UART_FC ) {
    return;
  }

  UART_FC_CLK_ENABLE();
  UART_FC_PIN_TX.enableClk();
  UART_FC_PIN_RX.enableClk();
  UART_FC_PIN_TX.cfgAF( UART_FC_GPIO_AF );
  UART_FC_PIN_RX.cfgAF( UART_FC_GPIO_AF );

  // HAL_NVIC_SetPriority(USART1_IRQn, 4, 0);
  // HAL_NVIC_EnableIRQ(USART1_IRQn);
  UVAR_x |= 2;
}

