#include <cstring>

#include <oxc_auto.h>

using namespace std;

void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);


// BOARD_DEFINE_LEDS;
BOARD_DEFINE_LEDS_EXTRA;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

extern "C" {

void task_leds( void *prm UNUSED_ARG );
void task_send( void *prm UNUSED_ARG );

}

UART_HandleTypeDef uah;
const int TX_BUF_SZ = 128;
char tx_buf[TX_BUF_SZ];
int prs( const char *s );

volatile uint32_t loop_delay = 1000;
const uint16_t    tx_wait = 100;

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  leds.write( 0x0A );  delay_bad_ms( 200 );

  MX_USART1_UART_Init();

  xTaskCreate( task_leds, "leds", 1*def_stksz, 0, 1, 0 );
  xTaskCreate( task_send, "send", 2*def_stksz, 0, 1, 0 );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

int prs( const char *s )
{
  if( !s || !*s) {
    return 0;
  }
  int l = strlen( s );
  int rc;
  if( ( rc = HAL_UART_Transmit( &uah, (uint8_t*)(s), l, tx_wait )) != HAL_OK ) {
    // leds.toggle( BIT3 );
    return 0;
  }
  return l;
}


void task_send( void *prm UNUSED_ARG )
{
  // strcpy( tx_buf, "ABCDE <.> 0123\r\n" );
  // int ssz = strlen( tx_buf );
  char buf[32];
  // uint8_t rc;
  int T_in_i = 28, T_in_p = 7, T_out_i = 301, T_out_p = 1; // 28.7 C, 301.1 C

  prs( "Start\r\n" );

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  while( 1 )
  {
    TickType_t tcc = xTaskGetTickCount();
    tx_buf[0] = '\0';
    i2dec( tcc - tc00, buf );  strncat( tx_buf, buf, 10 ); strncat( tx_buf, " ", 1 );
    i2dec( T_in_i, buf );      strncat( tx_buf, buf, 10 ); strncat( tx_buf, ".", 1 );
    i2dec( T_in_p, buf );      strncat( tx_buf, buf, 10 ); strncat( tx_buf, " ", 1 );
    i2dec( T_out_i, buf );     strncat( tx_buf, buf, 10 ); strncat( tx_buf, ".", 1 );
    i2dec( T_out_p, buf );     strncat( tx_buf, buf, 10 );
    strncat( tx_buf, "\r\n", 3 );
    prs( tx_buf );
    vTaskDelayUntil( &tc0, loop_delay );
  }
}



void Error_Handler(void)
{
  die4led( 0x00 );
}

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

