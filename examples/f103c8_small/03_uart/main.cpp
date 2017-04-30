#include <cstring>

#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
// BOARD_DEFINE_LEDS;
BOARD_DEFINE_LEDS_EXTRA;

void MX_GPIO_Init(void);
int MX_USART1_UART_Init(void);


extern "C" {
void task_leds( void *prm UNUSED_ARG );
void task_send( void *prm UNUSED_ARG );
} // extern "C"

UART_HandleTypeDef uah;

const int TX_BUF_SZ = 128;
char tx_buf[TX_BUF_SZ];

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 0;
  }

  delay_bad_ms( 200 );  leds.write( 0 );

  if( ! MX_USART1_UART_Init() ) {
      die4led( 1 );
  }

  leds.write( 0x01 );  delay_bad_ms( 200 );

  xTaskCreate( task_leds, "leds", 1*def_stksz, 0, 1, 0 );
  xTaskCreate( task_send, "send", 2*def_stksz, 0, 1, 0 );

  SCHEDULER_START;
  return 0;
}


void task_send( void *prm UNUSED_ARG )
{
  strcpy( tx_buf, "ABCDE <.> 0123\r\n" );
  //               0123456789ABCDEF1011
  int ssz = strlen( tx_buf );
  char c = '?';
  uint8_t rc;

  while( 1 ) {
    // leds.toggle( BIT2 );
    // really useless: delay_ms(1000) provokes overrun
    if( __HAL_UART_GET_FLAG( &uah, UART_FLAG_RXNE) ) {
      if( HAL_UART_Receive( &uah, (uint8_t*)&c, 1, 1 ) == HAL_OK ) {
        leds.toggle( BIT3 );
        tx_buf[7] = c;
      } else {
        uint8_t st = HAL_UART_GetState( &uah );
        if( st == HAL_UART_STATE_TIMEOUT ) {
          uah.State = HAL_UART_STATE_READY;
        } else {
          st <<= 1;
          leds.write( 0 );
          die4led( st );
        }
      }
    }

    if( (rc = HAL_UART_Transmit( &uah, (uint8_t*)tx_buf, ssz, 100 )) != HAL_OK ) {
      leds.toggle( BIT5 );
    }
    delay_ms( 1000 );
  }
}



// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

