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
    Error_Handler();
  }
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
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

  GPIO_InitStruct.Pin  = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
  // leds.toggle( BIT2 );
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if( uartHandle->Instance!=USART1 ) {
    return;
  }
  __HAL_RCC_USART1_CLK_DISABLE();
  HAL_GPIO_DeInit( GPIOA, GPIO_PIN_9|GPIO_PIN_10 );
}


// ---------------------------------------------------------

extern SPI_HandleTypeDef hspi1;

void MX_SPI1_Init()
{
  hspi1.Instance               = SPI1;
  hspi1.Init.Mode              = SPI_MODE_MASTER;
  hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
  hspi1.Init.NSS               = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode            = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial     = 10;
  if( HAL_SPI_Init( &hspi1 ) != HAL_OK ) {
    Error_Handler();
  }
}

void HAL_SPI_MspInit( SPI_HandleTypeDef* spiHandle )
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if( spiHandle->Instance !=SPI1 ) {
    return;
  }

  // __HAL_RCC_GPIOA_CLK_ENABLE(); // from USART1
  // __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_SPI1_CLK_ENABLE();

  // PA4   --> SPI1_NSS Manual
  // PA5   --> SPI1_SCK
  // PA6   --> SPI1_MISO
  // PA7   --> SPI1_MOSI

  GPIO_InitStruct.Pin   = GPIO_PIN_5 | GPIO_PIN_7;
  GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

  GPIO_InitStruct.Pin   = GPIO_PIN_6;
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
}

void HAL_SPI_MspDeInit( SPI_HandleTypeDef* spiHandle )
{
  if( spiHandle->Instance !=SPI1 ) {
    return;
  }
  __HAL_RCC_SPI1_CLK_DISABLE();

  HAL_GPIO_DeInit( GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 );
}

