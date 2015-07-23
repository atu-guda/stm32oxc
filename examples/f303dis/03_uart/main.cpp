#include <string.h>

#include <bsp/board_stm32f3discovery.h>
#include <oxc_gpio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


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

  // __HAL_RCC_USART2_CONFIG( RCC_USART2CLKSOURCE_SYSCLK );
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  leds.write( 0x0A );  delay_bad_ms( 200 );
  leds.write( 0x00 );

  init_uart( &uah );

  xTaskCreate( task_leds, "leds", 1*def_stksz, 0, 1, 0 );
  xTaskCreate( task_send, "send", 2*def_stksz, 0, 1, 0 );

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_leds( void *prm UNUSED_ARG )
{
  while (1)
  {
    leds.toggle( BIT0 );
    delay_ms( 500 );
  }
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

    if( (rc = HAL_UART_Transmit( &uah, (uint8_t*)tx_buf, ssz, 100 )) != HAL_OK ) {
      leds.toggle( BIT5 );
    }
    delay_ms( 1000 );
  }
}


void _exit( int rc )
{
  exit_rc = rc;
  for( ;; );
}

//  configs

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
  uahp->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if( HAL_UART_Init( uahp ) != HAL_OK )  {
    die4led( 0x08 );
  }
}


FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

