#include <oxc_hx711.h>

HX711::Ret HX711::read( HX711_mode mode )
{
  bool good { false };
  sck.reset();

  for( unsigned i=0; i<max_wait_rdy_mcs; ++i ) {
    uint16_t t = dat.read().bitmask();
    if( !t ) {
      good = true;
      break;
    }
    delay_hx();
  }
  if( !good ) { return { rc_notReady, 0 } ; };

  int32_t v { 0 };
  for( uint8_t i=0; i<ADC_BITS; ++i ) { // 24 bits - ADC value
    sck.set();
    delay_hx();
    uint16_t t = dat.read().bitmask();
    v <<= 1;
    if( t ) {
      v |= 1;
    }
    sck.reset();
    delay_hx();
  }

  const unsigned n_tick = mode + 1;
  for( unsigned i=0; i<n_tick; ++i ) {
    tick_sck();
  }

  if( v & 0x00800000 ) {
    v |=  0xFF000000;
  }

  return { rc_Ok, v };
}

