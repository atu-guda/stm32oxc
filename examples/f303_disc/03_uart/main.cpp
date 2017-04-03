#include <cstring>

#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;


// PinsOut p1 { GPIOE, 8, 8 };
BOARD_DEFINE_LEDS;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

extern "C" {

void task_leds( void *prm UNUSED_ARG );
void task_send( void *prm UNUSED_ARG );

}

UART_HandleTypeDef uah;
const int TX_BUF_SZ = 128;
char tx_buf[TX_BUF_SZ];
void init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  leds.write( 0x0A );  delay_bad_ms( 200 );
  leds.write( 0x00 );

  init_uart( &uah );
  HAL_NVIC_DisableIRQ( USART2_IRQn );

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
  strcpy( tx_buf, "ABCDE <.> 0123\r\n" );
  int ssz = strlen( tx_buf );
  char c = '?';
  uint8_t rc;

  while (1)
  {
    // leds.toggle( BIT2 );
    if( HAL_UART_Receive( &uah, (uint8_t*)&c, 1, 0 ) == HAL_OK ) {
      leds.toggle( BIT2 );
      tx_buf[7] = c;
    }
    if( HAL_UART_Transmit( &uah, (uint8_t*)tx_buf, ssz, 100 )!= HAL_OK ) {
      // leds.toggle( BIT3 );
    }
    delay_ms( 1000 );
  }
}


//  configs


FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc
