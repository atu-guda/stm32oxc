#include <cstring>

#include <oxc_auto.h>

using namespace std;



// PinsOut p1 { GPIOC, 0, 4 };
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
  init_uart( &uah );

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
  strcpy( tx_buf, "ABCDE <.> 0123\r\n" );
  int ssz = strlen( tx_buf );
  char c = '?';

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


void _exit( int rc )
{
  exit_rc = rc;
  for( ;; );
}


void init_uart( UART_HandleTypeDef *uahp, int baud )
{
  uahp->Instance = USART2;
  uahp->Init.BaudRate     = baud;
  uahp->Init.WordLength   = UART_WORDLENGTH_8B;
  uahp->Init.StopBits     = UART_STOPBITS_1;
  uahp->Init.Parity       = UART_PARITY_NONE;
  uahp->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uahp->Init.Mode         = UART_MODE_TX_RX;
  uahp->Init.OverSampling = UART_OVERSAMPLING_16;
  if( HAL_UART_Init( uahp ) != HAL_OK )  {
    die4led( 0x08 );
  }
}


FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

