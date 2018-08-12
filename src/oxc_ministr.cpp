#include <cstring>

#include "oxc_ministr.h"

using namespace std;



void MiniStr::append( char rhs )
{
  if( sz >= cap-1 ) {
    if( flush_fun ) {
      flush();
    } else {
      return;
    }
  }
  *e++ = rhs; *e = '\0'; ++sz;
}

bool MiniStr::ensureSpace( unsigned req )
{
  if( sz < cap - req ) {
    return true;
  }
  if( flush_fun ) {
    flush();
  }
  return sz < cap - req;
}

void MiniStr::add_bitnames( uint32_t b, const BitNames *bn )
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



MiniStr& MiniStr::operator+=( const char *rhs )
{
  if( !rhs ) {
    return *this;
  }

  while( *rhs ) { // long string, try by chars
    append( *rhs++ );
  }
  return *this;
}

MiniStr& MiniStr::operator+=( int rhs )
{
  if( ensureSpace( INT_STR_SZ_DEC ) ) {
    int n_add = i2dec_n( rhs, e );
    e += n_add; sz += n_add; *e = '\0';
  }
  return *this;
}

MiniStr& MiniStr::operator+=( HexInt rhs )
{
  if( ensureSpace( 9 ) ) { // XXXXXXXX
    word2hex( rhs, e );
    e += 8; sz += 8; *e = '\0';
  }
  return *this;
}

MiniStr& MiniStr::operator+=( HexInt8 rhs )
{
  if( ensureSpace( 3 ) ) { // XX
    char2hex( rhs, e );
    e += 2; sz += 2; *e = '\0';
  }
  return *this;
}

MiniStr& MiniStr::operator+=( HexInt16 rhs )
{
  if( ensureSpace( 5 ) ) { // XXXX
    short2hex( rhs, e );
    e += 4; sz += 4; *e = '\0';
  }
  return *this;
}

MiniStr& MiniStr::operator+=( const FmtInt &rhs )
{
  if( ensureSpace( INT_STR_SZ_DEC ) ) {
    int n_add = i2dec_n( rhs.v, e, rhs.min_sz, rhs.fill_ch );
    e += n_add; sz += n_add; *e = '\0';
  }
  return *this;
}

MiniStr& MiniStr::operator+=( FixedPoint1 rhs )
{
  if( ! ensureSpace( INT_STR_SZ_DEC + 2 ) ) {
    return *this;
  }
  int vi = rhs.toInt();
  if( vi < 0 ) {
    *e++ = '-'; ++sz; vi = -vi;
  }
  int n_add = i2dec_n( vi, e );
  e += n_add; sz += n_add;
  strcat( e, FixedPoint1::fracStr[ rhs.frac() ] );
  e += 2; sz += 2;
  *e = '\0';
  return *this;
}

MiniStr& MiniStr::operator+=( FixedPoint2 rhs )
{
  if( ! ensureSpace( INT_STR_SZ_DEC + 3 ) ) {
    return *this;
  }
  int vi = rhs.toInt();
  if( vi < 0 ) {
    *e++ = '-'; ++sz; vi = -vi;
  }
  int n_add = i2dec_n( vi, e );
  e += n_add; sz += n_add;
  strcat( e, FixedPoint2::fracStr[ rhs.frac() ] );
  e += 3; sz += 3;
  *e = '\0';
  return *this;
}

MiniStr& MiniStr::operator+=( FloatMult rhs )
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
  // i2dec( i1, t, min_sz_int,  int_fill );
  // strncat( s, t, INT_STR_SZ_DEC );
  append( '.' );
  operator+=( FmtInt( i2, rhs.min_sz_frac, '0' ) );
  // i2dec( i2, t, min_sz_frac, '0'     );
  // strncat( s, t, INT_STR_SZ_DEC );
  return *this;
}

MiniStr& MiniStr::operator+=( const BitsStr &rhs )
{
  add_bitnames( rhs.v, rhs.bn );
  return *this;
}

