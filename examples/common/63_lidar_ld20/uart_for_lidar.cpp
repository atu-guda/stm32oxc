#include <cerrno>
#include <oxc_auto.h>


DMA_HandleTypeDef hdma_usart_lidar_rx;
UART_HandleTypeDef huart_lidar;

// called from HAL_UART_MspInit if defined HAL_UART_USERINIT_FUN
// void HAL_UART_UserInit( UART_HandleTypeDef* uartHandle );

int MX_LIDAR_LD20_UART_Init(void)
{
  huart_lidar.Instance          = UART_LIDAR_LD20;
  huart_lidar.Init.BaudRate     = 230400;
  huart_lidar.Init.WordLength   = UART_WORDLENGTH_8B;
  huart_lidar.Init.StopBits     = UART_STOPBITS_1;
  huart_lidar.Init.Parity       = UART_PARITY_NONE;
  // huart_lidar.Init.Mode         = UART_MODE_RX;
  huart_lidar.Init.Mode         = UART_MODE_TX_RX;
  huart_lidar.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart_lidar.Init.OverSampling = UART_OVERSAMPLING_16;
  dbg_val3 = UART_LIDAR_LD20->USART_SR_REG;
  if( HAL_UART_Init( &huart_lidar ) != HAL_OK ) {
    return 0;
  }
  return 1;
}

void MX_LIDAR_LD20_DMA_Init(void)
{
  UART_LIDAR_LD20_DMA_CLK_ENABLE();
  HAL_NVIC_SetPriority( DMA_LIDAR_LD20_IRQ, 2, 0 );
  HAL_NVIC_EnableIRQ(   DMA_LIDAR_LD20_IRQ );
}

void HAL_UART_UserInit( UART_HandleTypeDef* uartHandle )
{
  if( uartHandle->Instance != UART_LIDAR_LD20 ) {
    return;
  }
  UART_LIDAR_LD20_GPIO.enableClk();

  UART_LIDAR_LD20_CLK_ENABLE();

  #if ! defined (STM32F1)
    UART_LIDAR_LD20_GPIO.cfgAF_N( UART_LIDAR_LD20_GPIO_PINS, UART_LIDAR_LD20_GPIO_AF );
  #else
    // UART_LIDAR_LD20_GPIO.cfgAF_N( UART_LIDAR_LD20_GPIO_TX, 1 );
    UART_LIDAR_LD20_GPIO.cfgIn_N( UART_LIDAR_LD20_GPIO_RX );
  #endif


  hdma_usart_lidar_rx.Instance                 = DMA_LIDAR_LD20_STREAM;
  #ifdef DMA_LIDAR_LD20_REQUEST
  hdma_usart_lidar_rx.Init.Request             = DMA_LIDAR_LD20_REQUEST;
  #else
  hdma_usart_lidar_rx.Init.Channel             = DMA_LIDAR_LD20_CHANNEL;
  #endif
  hdma_usart_lidar_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_usart_lidar_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_usart_lidar_rx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_usart_lidar_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart_lidar_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_usart_lidar_rx.Init.Mode                = DMA_CIRCULAR;
  hdma_usart_lidar_rx.Init.Priority            = DMA_PRIORITY_MEDIUM;
  hdma_usart_lidar_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  if( HAL_DMA_Init( &hdma_usart_lidar_rx ) != HAL_OK ) {
    errno = 3777;
    return; // Error!
  }

  __HAL_LINKDMA( uartHandle, hdmarx, hdma_usart_lidar_rx );

}


void LIDAR_LD20_DMA_IRQHandler(void)
{
  leds.toggle( 2 );
  ++UVAR('i');
  HAL_DMA_IRQHandler( &hdma_usart_lidar_rx );
}


