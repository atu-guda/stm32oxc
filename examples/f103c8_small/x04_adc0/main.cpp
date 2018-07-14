#include <cstring>

#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

UART_HandleTypeDef uah;
int out_uart( const char *d, unsigned n );

int out_uart( const char *d, unsigned n )
{
  return HAL_UART_Transmit( &uah, (uint8_t*)d, n, 10 ) == HAL_OK;
}

const int delay_val = 100;

int main(void)
{
  STD_PROLOG_UART_NOCON;

  leds.write( 0 );
  MSTRF( os, 128, out_uart );

  uint32_t c_msp = __get_MSP();
  os << " MSP-__heap_top = " << ((unsigned)c_msp - (unsigned)(__heap_top) ) << NL;
  os.flush();

  uint32_t tm0 = HAL_GetTick(), tmc = tm0;
  int v0, v1, v2, v3, vbin;

  for( int n=0; ; ++n ) {
    leds.toggle( BIT1 );

    v0 = 1000 + (n&0x0F) * 10; // Fake values for now
    v1 = 2000 + (n&0x07) * 20;
    v2 = 3000 + (n&0x07) * 30;
    v3 =  100 + (n&0x07) *  1;
    vbin = n & 0x0F;
    os << FmtInt( n * delay_val, 8, '0' ) << ' '
       << FloatMult( v0, 3 ) << ' '
       << FloatMult( v1, 3 ) << ' '
       << FloatMult( v2, 3 ) << ' '
       << FloatMult( v3, 3 ) << ' '
       << HexInt8( vbin )    << ' '
       << ( HAL_GetTick() - tmc ) << NL;
    os.flush();

    tmc += delay_val;
    uint32_t tm1 = HAL_GetTick();
    delay_ms( tmc - tm1 );
  }


  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

