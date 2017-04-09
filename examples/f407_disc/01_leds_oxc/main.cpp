#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;


BOARD_DEFINE_LEDS;

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

  int i=0x04;

  // write/set test
  leds.write( BOARD_LEDS_ALL );
  // delay_ms(  200 );
  HAL_Delay(  200 );
  leds.write( 0x00 );
  delay_ms(  100 );
  leds.set( 0x01 );
  delay_ms( 1000 );
  leds.set( 0x11 );
  delay_ms( 2000 );

  // reset test
  leds.write( BOARD_LEDS_ALL );
  delay_ms(  200 );
  leds.reset( 0x01 );
  delay_ms( 1000 );
  leds.reset( 0x08 );
  delay_ms( 1000 );

  // toggle test
  leds.write( BOARD_LEDS_ALL );
  delay_ms(  200 );
  leds.toggle( 0x02 );
  delay_ms( 1000 );
  leds.toggle( 0x01 );
  delay_ms( 1000 );

  // die4led( 0x0C );

  while (1)
  {
    leds.write( i );
    ++i;
    i &= BOARD_LEDS_ALL;
    delay_ms( 200 );
  }
  return 0;
}


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

