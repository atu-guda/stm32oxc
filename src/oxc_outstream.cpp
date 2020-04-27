#include <cstring>

#include <oxc_outstream.h>

using namespace std;

OutStream std_out( nullptr );


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
        HexInt( v ).out( *this );
      } else if ( bn->n > 8 ) {
        HexInt16( v ).out( *this );
      } else {
        HexInt8( v ).out( *this );
      }
    }
  }
  append( '}' );
}


OutStream& OutStream::operator+=( int rhs )
{
  char buf[INT_STR_SZ_DEC];
  i2dec_n( rhs, buf );
  append( buf );
  return *this;
}

void HexInt::out( OutStream &os ) const
{
  if( pr ) {
    os.append( "0x" );
  }
  char buf[12];  // 0xAABBCCDD
  word2hex( v, buf );
  os.append( buf );
}

void HexInt64::out( OutStream &os ) const
{
  if( pr ) {
    os.append( "0x" );
  }
  char buf[20];  // 0xAABBCCDDEEFFGGHH
  u64_2hex( v, buf );
  os.append( buf );
}

void HexInt8::out( OutStream &os ) const
{
  char buf[4];  // AA
  char2hex( v, buf );
  os.append( buf );
}

void HexInt16::out( OutStream &os ) const
{
  char buf[8];  // AABB
  short2hex( v, buf );
  os.append( buf );
}


void FmtInt::out( OutStream &os ) const
{
  char buf[INT_STR_SZ_DEC];
  i2dec_n( v, buf, min_sz, fill_ch );
  os.append( buf );
}

void FixedPoint1::out( OutStream &os ) const
{
  char buf[INT_STR_SZ_DEC+2];
  int vi = toInt();
  if( vi < 0 ) {
    os.append( '-' ); vi = -vi;
  }
  i2dec_n( vi, buf );
  os.append( buf );
  os.append(  FixedPoint1::fracStr[ frac() ] );
}

void FixedPoint2::out( OutStream &os ) const
{
  char buf[INT_STR_SZ_DEC+3];
  int vi = toInt();
  if( vi < 0 ) {
    os.append( '-' ); vi = -vi;
  }
  i2dec_n( vi, buf );
  os.append( buf );
  os.append( FixedPoint2::fracStr[ frac() ] );
}

void FloatMult::out( OutStream &os ) const
{
  int i1 = v / mult;
  int i2 = v - i1 * mult;
  char sig = '-';
  if( i1 >=0  &&  i2 >= 0 ) {
    sig = plus_ch;
  }
  if( i1 < 0 ) { i1 = -i1; }
  if( i2 < 0 ) { i2 = -i2; }
  os.append( sig );
  char int_fill = ' ';
  if( min_sz_int > 1 ) { int_fill = '0'; };

  os << FmtInt( i1, min_sz_int, int_fill );
  os.append( '.' );
  os << FmtInt( i2, min_sz_frac, '0' );
}


void BitsStr::out( OutStream &os ) const
{
  os.add_bitnames( v, bn );
}


