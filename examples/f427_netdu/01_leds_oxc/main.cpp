#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;



BOARD_DEFINE_LEDS;

BOARD_DEFINE_GO_LEDS;
BOARD_DEFINE_GO_PWR;
// To test E0/E1 connections
// PinsOut e_pins( GPIOE, 0, 3 );

int main(void)
{
  STD_PROLOG_START;

  led_go1.initHW();  led_go2.initHW();  led_go3.initHW();
  pwr_go1.initHW();  pwr_go2.initHW();  pwr_go3.initHW();


  int i=0x04;

  // write/set test
  leds.write( BOARD_LEDS_ALL );
  led_go1.write( 1 );  led_go2.write( 1 );  led_go3.write( 1 );
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
  leds.write( BOARD_LEDS_ALL );
  delay_ms(  200 );
  leds.reset( 0x0A );
  delay_ms( 1000 );
  leds.reset( 0x01 );
  delay_ms( 1000 );

  // toggle test
  leds.write( BOARD_LEDS_ALL );
  delay_ms(  200 );
  leds.toggle( 0x0A );
  delay_ms( 1000 );
  leds.toggle( 0x08 );
  delay_ms( 1000 );

  // die4led( 0x0C );
  led_go1.write( 0 );  led_go2.write( 0 );  led_go3.write( 0 );
  pwr_go1.write( 1 );  pwr_go2.write( 1 );  pwr_go3.write( 1 );

  while(1) {
    leds.write( i );
    if( i & 0x07 ) {
      led_go1.toggle( BIT0 );
    }
    if( ( i & 0x07 ) == 3 ) {
      led_go1.toggle( BIT0 );
    }
    if( ( i & 0x0F ) == 0 ) {
      pwr_go3.toggle( BIT0 );
    }

    // e_pins.write( i );

    ++i;
    i &= BOARD_LEDS_ALL;
    delay_ms( 200 );
  }
  return 0;
}

// configs

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

