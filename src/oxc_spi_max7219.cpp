#include <oxc_spi_max7219.h>

void DevSPI_MAX7219::setDigits( const uint8_t *vs, uint8_t dpos, uint8_t st, uint8_t en )
{
  if( !vs || st > 7 || st > en ) { return; }
  for( uint8_t i=st, j=0; i<=en; ++i,++j ) {
    uint8_t v = vs[j];
    if( i == dpos ) { // decimal point
      v |= 0x80;
    }
    setDigit( i, v );
  }
}

void DevSPI_MAX7219::setXDigit( uint8_t pos, uint8_t v )
{
  if( pos > 7 ) { return; };
  uint8_t dot = v & 0x80;
  v &= 0x7F;
  const constexpr int n_xdig = 22;
  if( v >= n_xdig ) { v = n_xdig-1; } // 0-F - = || -=
  static const uint8_t cvt[n_xdig] = {
  //   0     1     2     3     4     5     6     7
    0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70,
  //   8     9     A     b     C     d     E     F
    0x7F, 0x7B, 0x77, 0x1F, 0x4E, 0x3d, 0x4F, 0x47,
  //   -    ' '    =    ||     _    -=
    0x01, 0x00, 0x41, 0x36, 0x08, 0x49
  };
  uint8_t v1 = cvt[v] | dot;
  setDigit( pos, v1 ); // assume bitmap
}

void DevSPI_MAX7219::setXDigits( const uint8_t *vs, uint8_t dpos, uint8_t st, uint8_t en )
{
  if( !vs || st > 7 || st > en ) { return; }
  for( uint8_t i=st, j=0; i<=en; ++i,++j ) {
    uint8_t v = vs[j];
    if( i == dpos ) { // decimal point
      v |= 0x80;
    }
    setXDigit( i, v );
  }
}


void DevSPI_MAX7219::setUVal( unsigned v, uint8_t dpos, uint8_t st, uint8_t l )
{
  uint8_t b[8]; // fixed number of places
  if( st > 7 ) { return; }
  if( st + l > 7 ) { l = 8 - st; };

  for( uint8_t i=0; i<l; ++i ) {
    b[i] = v % 10;
    v /= 10;
  }
  if( v > 0 ) { // overflow
    b[0] = 0x0B;
  }

  setDigits( b, dpos+st, st, st+l-1 );
}

void DevSPI_MAX7219::setVal( int v, uint8_t dpos, uint8_t st, uint8_t l )
{
  if( v >= 0 ) {
    setUVal( v, dpos, st, l );
    return;
  }
  unsigned v0 = -v;
  if( st > 7 ) { return; }
  if( st + l > 7 ) { l = 8 - st; };
  setDigit( st + l - 1, 0x0A );
  setUVal( v0, dpos, st, l-1 );
}

void DevSPI_MAX7219::setXVal( unsigned v, uint8_t dpos, uint8_t st, uint8_t l )
{
  uint8_t b[8]; // fixed number of places
  if( st > 7 ) { return; }
  if( st + l > 7 ) { l = 8 - st; };

  for( uint8_t i=0; i<l; ++i ) {
    b[i] = v & 0x0F;
    v >>= 4;
  }
  if( v > 0 ) { // overflow
    b[0] = 21;
  }

  setXDigits( b, dpos+st, st, st+l-1 );
}

void DevSPI_MAX7219::setSameVal( uint8_t st, uint8_t l, uint8_t val )
{
  if( st > 7 ) { return; };
  uint8_t enx = st + l;
  if( enx > 8 ) { enx=8; };
  for( uint8_t i=st; i<enx; ++i ) {
    setDigit( i, val );
  }
}

