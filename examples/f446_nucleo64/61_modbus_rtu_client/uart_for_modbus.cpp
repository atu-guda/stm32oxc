#include <oxc_auto.h>

// using namespace std;

// TMP
extern const unsigned ibuf_sz;
extern char ibuf[];
extern uint16_t ibuf_pos;
extern uint16_t ibuf_t[];

UART_HandleTypeDef huart_modbus;

// called from HAL_UART_MspInit if defined HAL_UART_USERINIT_FUN
void HAL_UART_UserInit( UART_HandleTypeDef* uartHandle );

int MX_MODBUS_UART_Init(void)
{
  huart_modbus.Instance          = UART_MODBUS;
  huart_modbus.Init.BaudRate     = 115200;
  huart_modbus.Init.WordLength   = UART_WORDLENGTH_8B;
  huart_modbus.Init.StopBits     = UART_STOPBITS_1;
  huart_modbus.Init.Parity       = UART_PARITY_NONE;
  huart_modbus.Init.Mode         = UART_MODE_TX_RX;
  huart_modbus.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart_modbus.Init.OverSampling = UART_OVERSAMPLING_16;
  if( HAL_UART_Init( &huart_modbus ) != HAL_OK ) {
    return 0;
  }
  UART_MODBUS->CR1 |=  UsartIO::CR1_UE  | UsartIO::CR1_RE | UsartIO::CR1_TE | UsartIO::CR1_RXNEIE;
  return 1;
}

void HAL_UART_UserInit( UART_HandleTypeDef* uartHandle )
{
  if( uartHandle->Instance != UART_MODBUS ) {
    return;
  }

  UART_MODBUS_CLK_ENABLE();

  #if ! defined (STM32F1)
    UART_MODBUS_GPIO.cfgAF_N( UART_MODBUS_GPIO_PINS, UART_MODBUS_GPIO_AF );
  #else
    UART_MODBUS_GPIO.cfgAF_N( UART_MODBUS_GPIO_TX, 1 );
    UART_MODBUS_GPIO.cfgIn_N( UART_MODBUS_GPIO_RX );
  #endif

  HAL_NVIC_SetPriority( UART_MODBUS_IRQ, 3, 0 );
  HAL_NVIC_EnableIRQ(   UART_MODBUS_IRQ );
}

void UART_MODBUS_IRQHANDLER(void)
{
  // leds.set( 2 );
  int n_work = 0;
  leds.toggle( 4 );
  uint32_t status = UART_MODBUS->USART_SR_REG;
  UVAR('s') = status;
  ++UVAR('i');

  if( status & UART_FLAG_RXNE ) { // char received
    ++UVAR('j');
    leds.set( BIT1 );
    ++n_work;
    char cr = UART_MODBUS->USART_RX_REG & (uint16_t)0x0FF;
    // TODO: trylock

    if( status & ( UART_FLAG_ORE | UART_FLAG_FE /*| UART_FLAG_LBD*/ ) ) { // TODO: on MCU
      UVAR('e') = status;
      // reset
    } else {
      if( ibuf_pos < 256-2 ) { // TODO: real
        ibuf_t[ibuf_pos] = TIM11->CNT;
        ibuf[ibuf_pos++] = cr;
      }
    }
    leds.reset( BIT1 );
  }
  if( n_work == 0 ) { // unhandled
    leds.set( BIT0 );
  }

}



