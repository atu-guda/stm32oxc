#include <bit>

#include "oxc_tmc2209.h"

uint8_t TMC2209::calc_crc( const uint8_t *d, unsigned sz )
{
  uint8_t crc = 0;
  for( unsigned i=0; i<sz; ++i ) {
    uint8_t c = d[i];
    for( uint8_t j=0; j<8; ++j ) {
      if( ( crc >> 7) ^ ( c & 0x01 ) ) {
        crc = ( crc << 1 ) ^ 0x07;
      } else {
        crc = ( crc << 1 );
      }
      c >>= 1;
    }
  }
  return crc;
}


void TMC2209::rwdata::fill( uint8_t dev_addr, uint8_t reg, uint32_t dat )
{
  sync = 0x05;
  addr = dev_addr;
  regnum = reg | 0x80;
  data = __builtin_bswap32( dat );
  crc = calc_crc( rawCData(), sizeof(*this)-1 );
}

void TMC2209::rreq::fill( uint8_t dev_addr, uint8_t reg )
{
  sync = 0x05;
  addr = dev_addr;
  regnum = reg;
  crc = calc_crc( rawCData(), sizeof(*this)-1  );
}

