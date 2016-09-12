#include <oxc_auto.h>

using namespace std;

// void MX_GPIO_Init(void);
extern "C" {
void Error_Handler(void);
};


// PinsOut p1 { GPIOB, 12, 4 };
// BOARD_DEFINE_LEDS;
BOARD_DEFINE_LEDS_EXTRA;

int main(void)
{
  HAL_Init();

  SystemClock_Config();

//  MX_GPIO_Init();

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
    // delay_ms( 128 );
    delay_ms( 10 );
  }
  return 0;
}

void _exit( int rc )
{
  exit_rc = rc;
  for( ;; );
}

void Error_Handler(void)
{
  while(1) {
  }
}

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

