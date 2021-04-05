#include <oxc_auto.h>

using namespace std;

DMA_HandleTypeDef hdma_usart_ut61e_rx;
UART_HandleTypeDef huart_ut61e;

// called from HAL_UART_MspInit if defined HAL_UART_USERINIT_FUN
void HAL_UART_UserInit( UART_HandleTypeDef* uartHandle );

int MX_UT61E_UART_Init(void)
{
  huart_ut61e.Instance          = UART_UT61E;
  huart_ut61e.Init.BaudRate     = 19200;
  huart_ut61e.Init.WordLength   = UART_WORDLENGTH_8B;
  huart_ut61e.Init.StopBits     = UART_STOPBITS_1;
  huart_ut61e.Init.Parity       = UART_PARITY_ODD;
  huart_ut61e.Init.Mode         = UART_MODE_TX_RX;
  huart_ut61e.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart_ut61e.Init.OverSampling = UART_OVERSAMPLING_16;
  if( HAL_UART_Init( &huart_ut61e ) != HAL_OK ) {
    return 0;
  }
  return 1;
}

void MX_UT61E_DMA_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  HAL_NVIC_SetPriority( DMA_UT61E_IRQ, 2, 0 );
  HAL_NVIC_EnableIRQ(   DMA_UT61E_IRQ );
}

void HAL_UART_UserInit( UART_HandleTypeDef* uartHandle )
{
  if( uartHandle->Instance != UART_UT61E ) {
    return;
  }

  UART_UT61E_CLK_ENABLE();

  #if ! defined (STM32F1)
    UART_UT61E_GPIO.cfgAF_N( UART_UT61E_GPIO_PINS, UART_UT61E_GPIO_AF );
  #else
    UART_UT61E_GPIO.cfgAF_N( UART_UT61E_GPIO_TX, 1 );
    UART_UT61E_GPIO.cfgIn_N( UART_UT61E_GPIO_RX );
  #endif


  hdma_usart_ut61e_rx.Instance                 = DMA_UT61E_STREAM;
  hdma_usart_ut61e_rx.Init.Channel             = DMA_UT61E_CHANNEL;
  hdma_usart_ut61e_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_usart_ut61e_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_usart_ut61e_rx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_usart_ut61e_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart_ut61e_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_usart_ut61e_rx.Init.Mode                = DMA_CIRCULAR;
  // hdma_usart_ut61e_rx.Init.Mode                = DMA_NORMAL;
  hdma_usart_ut61e_rx.Init.Priority            = DMA_PRIORITY_MEDIUM;
  hdma_usart_ut61e_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  if( HAL_DMA_Init( &hdma_usart_ut61e_rx ) != HAL_OK ) {
    return; // Error!
  }

  // HAL_DMA_RegisterCallback( &hdma_usart_ut61e_rx, HAL_DMA_XFER_CPLT_CB_ID, DMA_CC );

  __HAL_LINKDMA( uartHandle, hdmarx, hdma_usart_ut61e_rx );

  HAL_NVIC_SetPriority( USART1_IRQn, 4, 0 ); // TODO: ??? + define
  HAL_NVIC_EnableIRQ( USART1_IRQn );
}

void USART1_IRQHandler(void)
{
  // leds.set( 2 );
  HAL_UART_IRQHandler( &huart_ut61e );
}

void DMA2_Stream2_IRQHandler(void)
{
  leds.toggle( 2 );
  HAL_DMA_IRQHandler( &hdma_usart_ut61e_rx );
}


