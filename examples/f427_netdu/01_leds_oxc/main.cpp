#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;



BOARD_DEFINE_LEDS;
const uint32_t leds_all = BOARD_LEDS_ALL;

BOARD_DEFINE_GO_LEDS;
BOARD_DEFINE_GO_PWR;

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL );

  led_go1.initHW();  led_go2.initHW();  led_go3.initHW();
  pwr_go1.initHW();  pwr_go2.initHW();  pwr_go3.initHW();

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 0;
  }


  int i=0x04;

  // write/set test
  leds.write( leds_all );
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
  led_go1.write( 0 );  led_go2.write( 0 );  led_go3.write( 0 );
  pwr_go1.write( 1 );  pwr_go2.write( 1 );  pwr_go3.write( 1 );

  while(1) {
    leds.write( i );
    led_go1.toggle( BIT0 );
    pwr_go3.toggle( BIT0 );
    ++i;
    i &= leds_all;
    delay_ms( 200 );
  }
  return 0;
}


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

