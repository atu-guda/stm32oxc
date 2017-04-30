#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;



BOARD_DEFINE_LEDS;
const uint32_t leds_all = BOARD_LEDS_ALL;

int main(void)
{
  STD_PROLOG_START;

  int i=0x04;

  // write/set test
  leds.write( leds_all );
  // delay_ms(  200 );
  HAL_Delay(  500 );
  leds.write( 0x00 );
  delay_ms(  100 );
  leds.set( 0x0A );
  delay_ms( 1000 );
  leds.set( 0x01 );
  delay_ms( 1000 );
  leds.set( 0x11 );
  delay_ms( 2000 );

  // reset test
  leds.write( leds_all );
  delay_ms(  200 );
  leds.reset( 0x0A );
  delay_ms( 1000 );
  leds.reset( 0x01 );
  delay_ms( 1000 );

  // toggle test
  leds.write( leds_all );
  delay_ms(  200 );
  leds.toggle( 0x0A );
  delay_ms( 1000 );
  leds.toggle( 0x08 );
  delay_ms( 1000 );

  // die4led( 0x0C );

  while(1) {
    leds.write( i );
    ++i;
    i &= leds_all;
    delay_ms( 200 );
  }
  return 0;
}

// configs

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

