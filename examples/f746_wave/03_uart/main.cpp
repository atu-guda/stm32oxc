#include <cstring>

#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;



// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

extern "C" {

void task_leds( void *prm UNUSED_ARG );
void task_send( void *prm UNUSED_ARG );

}

UART_HandleTypeDef huart1;
const int TX_BUF_SZ = 128;
char tx_buf[TX_BUF_SZ];
void MX_USART1_UART_Init(void);

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  leds.initHW();
  MX_USART1_UART_Init();

  leds.write( 0x0F );  delay_bad_ms( 200 );

  xTaskCreate( task_leds, "leds", 1*def_stksz, 0, 1, 0 );
  xTaskCreate( task_send, "send", 2*def_stksz, 0, 1, 0 );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}


void task_send( void *prm UNUSED_ARG )
{
  strcpy( tx_buf, "ABC <.> ------\r\n" );
  //               0123456789ABCDEF1011
  int ssz = strlen( tx_buf );
  char c = '?', cn = '0';

  while( 1 ) {
    leds.toggle( BIT1 );
    tx_buf[5]  = ' ';
    tx_buf[9] = cn; tx_buf[11] = '.';
    ++cn;
    if( cn >= 0x7F ) { cn = ' '; }
    // if( HAL_UART_Receive( &huart1, (uint8_t*)&c, 1, 0 ) == HAL_OK ) {
    //   leds.toggle( BIT2 );
    //   tx_buf[5]  = c; tx_buf[11] = 'R';
    // }
    if( huart1.Instance->USART_SR_REG & UART_FLAG_RXNE ) {
      c = huart1.Instance->USART_RX_REG;
      leds.toggle( BIT2 );
      tx_buf[5]  = c; tx_buf[11] = 'R';
    }
    if( huart1.Instance->USART_SR_REG & UART_FLAG_ORE ) { // overrun
      huart1.Instance->ICR |= UART_CLEAR_OREF;
      c = huart1.Instance->USART_RX_REG;
      tx_buf[11] = 'O';
      leds.toggle( BIT0 );
    }
    if( HAL_UART_Transmit( &huart1, (uint8_t*)tx_buf, ssz, 100 )!= HAL_OK ) {
      leds.toggle( BIT0 );
    }
    delay_ms( 500 );
  }
}


void _exit( int rc )
{
  die4led( rc );
}

// // configs
FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

