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
      *e++ = rhs; *e = '\0'; ++sz;
    }
  } else {
    *e++ = rhs; *e = '\0'; ++sz;
  }
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



MiniStr& MiniStr::operator+=( const char *rhs )
{
  if( !rhs || !*rhs ) {
    return *this;
  }

  unsigned l = strlen( rhs ); // may be overkill, but try one peace
  if( ensureSpace( l+1 ) ) {
    strcat( e, rhs ); e += l; sz += l;
    return *this;
  }

  while( *rhs ) { // long string, try by chars
    append( *rhs++ );
  }
  return *this;
}

MiniStr& MiniStr::operator+=( int rhs )
{
  if( ! ensureSpace( INT_STR_SZ_DEC ) ) {
    return *this;
  }
  int n_add = i2dec_n( rhs, e );
  e += n_add; sz += n_add; *e = '\0';
  return *this;
}

MiniStr& MiniStr::operator+=( HexInt rhs )
{
  if( ! ensureSpace( 9 ) ) { // XXXXXXXX
    return *this;
  }
  word2hex( rhs, e );
  e += 8; sz += 8; *e = '\0';
  return *this;
}

MiniStr& MiniStr::operator+=( HexInt16 rhs )
{
  if( ! ensureSpace( 5 ) ) { // XXXX
    return *this;
  }
  short2hex( rhs, e );
  e += 4; sz += 4; *e = '\0';
  return *this;
}

MiniStr& MiniStr::operator+=( const FmtInt &rhs )
{
  if( ! ensureSpace( INT_STR_SZ_DEC ) ) {
    return *this;
  }
  int n_add = i2dec_n( rhs.v, e, rhs.min_sz, rhs.fill_ch );
  e += n_add; sz += n_add; *e = '\0';
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
  if( ! ensureSpace( INT_STR_SZ_DEC + 4 ) ) {
    return *this;
  }
  int vi = rhs.toInt();
  if( vi < 0 ) {
    *e++ = '-'; ++sz; vi = -vi;
  }
  int n_add = i2dec_n( vi, e );
  e += n_add; sz += n_add;
  strcat( e, FixedPoint2::fracStr[ rhs.frac() ] );
  e += 4; sz += 4;
  *e = '\0';
  return *this;
}

