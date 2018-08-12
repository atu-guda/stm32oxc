#include <cstring>

#include <oxc_outstream.h>

using namespace std;


void OutStream::add_bitnames( uint32_t b, const BitNames *bn )
{
  static int constexpr bpi = sizeof(b)*8;
  if( !bn ) { return; }
  append( '{' );
  char sep = 0;
  for( ; bn->n !=0 && bn->name != nullptr; ++bn ) {
    if( bn->n == 1 ) { // single bit
      if( b & (1<<bn->s) ) {
        append( sep ); sep = ',';
        operator+=( bn->name );
      }
    } else {           // pack of bits
      if( sep ) {
        append( sep );
      } else {
        sep = ',';
      }
      operator+=( bn->name ); append( '_' );
      uint32_t v = (b>>bn->s) & ( ~0u>>(bpi-bn->n) );
      if( bn->n > 16 ) {
        operator+=( HexInt( v ) );
      } else if ( bn->n > 8 ) {
        operator+=( HexInt16( v ) );
      } else {
        operator+=( HexInt8( v ) );
      }
    }
  }
  append( '}' );
}


OutStream& OutStream::operator+=( int rhs )
{
  char buf[INT_STR_SZ_DEC];
  i2dec_n( rhs, buf );
  operator+=( buf );
  return *this;
}

OutStream& OutStream::operator+=( HexInt rhs )
{
  char buf[9];  // AABBCCDD
  word2hex( rhs, buf );
  operator+=( buf );
  return *this;
}

OutStream& OutStream::operator+=( HexInt8 rhs )
{
  char buf[3];  // AA
  char2hex( rhs, buf );
  operator+=( buf );
  return *this;
}

OutStream& OutStream::operator+=( HexInt16 rhs )
{
  char buf[5];  // AABB
  short2hex( rhs, buf );
  operator+=( buf );
  return *this;
}

OutStream& OutStream::operator+=( const FmtInt &rhs )
{
  char buf[INT_STR_SZ_DEC];
  i2dec_n( rhs.v, buf, rhs.min_sz, rhs.fill_ch );
  operator+=( buf );
  return *this;
}

OutStream& OutStream::operator+=( FixedPoint1 rhs )
{
  char buf[INT_STR_SZ_DEC+2];
  int vi = rhs.toInt();
  if( vi < 0 ) {
    append( '-' ); vi = -vi;
  }
  i2dec_n( vi, buf );
  operator+=( buf );
  operator+=( FixedPoint1::fracStr[ rhs.frac() ] );
  return *this;
}

OutStream& OutStream::operator+=( FixedPoint2 rhs )
{
  char buf[INT_STR_SZ_DEC+3];
  int vi = rhs.toInt();
  if( vi < 0 ) {
    append( '-' ); vi = -vi;
  }
  i2dec_n( vi, buf );
  operator+=( buf );
  operator+=( FixedPoint2::fracStr[ rhs.frac() ] );
  return *this;
}

OutStream& OutStream::operator+=( FloatMult rhs )
{
  int i1 = rhs.v / rhs.mult;
  int i2 = rhs.v - i1 * rhs.mult;
  char sig = '-';
  if( i1 >=0  &&  i2 >= 0 ) {
    sig = rhs.plus_ch;
  }
  if( i1 < 0 ) { i1 = -i1; }
  if( i2 < 0 ) { i2 = -i2; }
  append( sig );
  char int_fill = ' ';
  if( rhs.min_sz_int > 1 ) { int_fill = '0'; };

  operator+=( FmtInt( i1, rhs.min_sz_int, int_fill ) );
  append( '.' );
  operator+=( FmtInt( i2, rhs.min_sz_frac, '0' ) );
  return *this;
}

OutStream& OutStream::operator+=( const BitsStr &rhs )
{
  add_bitnames( rhs.v, rhs.bn );
  return *this;
}

