#include <oxc_auto.h>

void MX_inp_Init()
{
  // __HAL_RCC_GPIOA_CLK_ENABLE(); // copy from UART
  // __HAL_RCC_AFIO_CLK_ENABLE();
  GpioA.cfgIn_N( GPIO_PIN_1 | GPIO_PIN_2, GpioRegs::Pull::down );
}

extern UART_HandleTypeDef uah_console;

/* USART1 init function */

int MX_USART1_UART_Init(void)
{
  uah_console.Instance          = USART1;
  uah_console.Init.BaudRate     = 115200;
  uah_console.Init.WordLength   = UART_WORDLENGTH_8B;
  uah_console.Init.StopBits     = UART_STOPBITS_1;
  uah_console.Init.Parity       = UART_PARITY_NONE;
  uah_console.Init.Mode         = UART_MODE_TX_RX;
  uah_console.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uah_console.Init.OverSampling = UART_OVERSAMPLING_16;
  return( HAL_UART_Init( &uah_console ) == HAL_OK );
}

void HAL_UART_MspInit( UART_HandleTypeDef* uartHandle )
{
  if( uartHandle->Instance != USART1 ) {
    return;
  }
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();

  //  PA9  ---> USART1_TX
  //  PA10 ---> USART1_RX
  GpioA.cfgAF( 9, 1 );
  GpioA.cfgIn( 10 );
  // leds.toggle( BIT2 );
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if( uartHandle->Instance != USART1 ) {
    return;
  }
  __HAL_RCC_USART1_CLK_DISABLE();
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
    Error_Handler( 2 );
  }
}

void HAL_SPI_MspInit( SPI_HandleTypeDef* spiHandle )
{

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

  GpioA.cfgAF_N( GPIO_PIN_5 | GPIO_PIN_7, 1 );
  GpioA.cfgIn( 6 );
  GpioA.cfgOut( 4 );
}

void HAL_SPI_MspDeInit( SPI_HandleTypeDef* spiHandle )
{
  if( spiHandle->Instance !=SPI1 ) {
    return;
  }
  __HAL_RCC_SPI1_CLK_DISABLE();
}

