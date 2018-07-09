#include <cstring>

#include "oxc_ministr.h"

using namespace std;

const char * const FixedPoint1::fracStr[4] = { ".0", ".5", ".?", ".X" };
const char * const FixedPoint2::fracStr[6] = { ".00", ".25", ".50", ".75", ".??", ".XX" };



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
  if( !rhs || !*rhs ) {
    return *this;
  }

  // unsigned l = strlen( rhs ); // may be overkill, but try one chunk
  // if( ensureSpace( l+1 ) ) {
  //   strcat( e, rhs ); e += l; sz += l;
  //   return *this;
  // }

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

MiniStr& MiniStr::operator+=( const BitsStr &rhs )
{
  add_bitnames( rhs.v, rhs.bn );
  return *this;
}

