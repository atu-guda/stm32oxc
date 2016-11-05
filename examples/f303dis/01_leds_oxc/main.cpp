#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;

void MX_GPIO_Init(void);


// PinsOut p1 { GPIOE, 8, 8 };
BOARD_DEFINE_LEDS;

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  leds.initHW();

  int i=0x04;

  // write/set test
  leds.write( 0x0F );
  delay_ms(  200 );
  leds.write( 0x00 );
  delay_ms(  100 );
  leds.set( 0x0A );
  delay_ms( 1000 );
  leds.set( 0x01 );
  delay_ms( 2000 );

  // reset test
  leds.write( 0x0F );
  delay_ms(  200 );
  leds.reset( 0x0A );
  delay_ms( 1000 );
  leds.reset( 0x01 );
  delay_ms( 1000 );

  // toggle test
  leds.write( 0x0F );
  delay_ms(  200 );
  leds.toggle( 0x0A );
  delay_ms( 1000 );
  leds.toggle( 0x08 );
  delay_ms( 1000 );

  // die4led( 0x0C );

  while (1)
  {
    leds.write( i );
    ++i;
    i &= 0xFF;
    delay_ms( 128 );
  }
  return 0;
}

void _exit( int rc )
{
  exit_rc = rc;
  for( ;; );
}

// configs
void MX_GPIO_Init(void)
{

  // GPIO_InitTypeDef GPIO_InitStruct;
  // moved to PinsOut initHW

  /*Configure GPIO pins : PA0 PA1 */
  // GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  // GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  // GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

