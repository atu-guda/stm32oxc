#include <oxc_auto.h>
#include <oxc_main.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;



BOARD_DEFINE_LEDS;

int main(void)
{
  STD_PROLOG_START;


  // write/set test
  leds.write( 0xFF_mask );
  // delay_ms(  200 );
  HAL_Delay(  500 );
  leds.write( 0x00_mask );
  delay_ms(  100 );
  leds.set( 0x0A_mask );
  delay_ms( 1000 );
  leds.set( 0x01_mask );
  delay_ms( 1000 );
  leds.set( 0x11_mask );
  delay_ms( 2000 );

  // reset test
  leds.write( 0xFF_mask );
  delay_ms(  200 );
  leds.reset( 0x0A_mask );
  delay_ms( 1000 );
  leds.reset( 0x01_mask );
  delay_ms( 1000 );

  // toggle test
  leds.write( 0xFF_mask );
  delay_ms(  200 );
  leds.toggle( 0x0A_mask );
  delay_ms( 1000 );
  leds.toggle( 0x08_mask );
  delay_ms( 1000 );


  for( unsigned i=0; i<64; ++i ) {
    leds.write( PinMask( i ) );
    // i &= BOARD_LEDS_ALL;
    delay_ms( 200 );
  }

  // PinOut pin( BOARD_LEDS_GPIO, BOARD_LEDS_OFS );

  for(;;) { // speed measure
    leds.write( 0xFF_mask );
    leds.write( 0x00_mask );
    // leds.toggle( 0xFF_mask );
    // pin.set();
    // pin.reset();
    // pin.write( true );
    // pin.write( false );
  }

  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

