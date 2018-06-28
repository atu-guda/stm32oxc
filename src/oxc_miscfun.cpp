#include <cstdlib>
#include <cstring>

#include <oxc_miscfun.h>

using namespace std;

uint8_t numFirstBit( uint32_t a )
{
  for( uint8_t n = 0; n<sizeof(a)*8; ++n ) {
    if( a & 1 ) {
      return n;
    }
    a >>= 1;
  }
  return 0xFF;
}

const char hex_digits[] = "0123456789ABCDEFG";
const char dec_digits[] = "0123456789???";


char* char2hex( char c, char *s )
{
  if( s != 0 ) {
    s[0] = hex_digits[ (uint8_t)(c) >> 4 ];
    s[1] = hex_digits[ c & 0x0F ];
    s[2] = 0;
  }
  return s;
}

char* word2hex( uint32_t d,  char *s )
{
  if( s != 0 ) {
    int i;
    for( i=7; i>=0; --i ) {
      s[i] = hex_digits[ d & 0x0F ];
      d >>= 4;
    }
    s[8] = 0;
  }
  return s;
}

char* short2hex( uint16_t d,  char *s )
{
  if( !s ) {
    return 0;
  }

  int i;
  for( i=4; i>=0; --i ) {
    s[i] = hex_digits[ d & 0x0F ];
    d >>= 4;
  }
  s[4] = 0;
  return s;
}


unsigned i2dec_n( int n, char *s, unsigned min_sz, char fill_ch )
{
  char tbuf[INT_STR_SZ_DEC];
  if( !s ) {
    return 0;
  }
  if( min_sz < 1 ) { min_sz = 1; }
  if( min_sz > INT_STR_SZ_DEC-2 ) { min_sz = INT_STR_SZ_DEC-2; }

  char *bufptr = s, *tmpptr = tbuf + 1;
  *tbuf = '\0';
  unsigned u, nc = 0;

  if( n < 0 ){ //  sign
    u = ( (unsigned)(-(1+n)) ) + 1; // INT_MIN handling
    *bufptr++ = '-'; ++nc;
  } else {
    u=n;
  }

  do {
    *tmpptr++ = dec_digits[ u % 10 ]; ++nc;
  } while( u /= 10 );

  while( nc < min_sz ) {
    *tmpptr++ = fill_ch; ++nc;
  }

  while( ( *bufptr++ = *--tmpptr ) != '\0' ) {
    /* NOP */
  }

  return nc;
}

char* i2dec( int n, char *s, unsigned min_sz, char fill_ch )
{
  static char sbuf[INT_STR_SZ_DEC];
  if( !s ) {
    s = sbuf;
  }
  i2dec_n( n, s, min_sz, fill_ch );
  return s;
}


char* ifcvt( int v, int mult, char *s, unsigned min_sz_frac,  unsigned min_sz_int, char plus_ch  )
{
  char t[INT_STR_SZ_DEC];
  if( !s ) {
    return 0;
  }
  int i1 = v / mult;
  int i2 = v - i1 * mult;
  char sig = '-';
  if( i1 >=0  &&  i2 >= 0 ) {
    sig = plus_ch;
  }
  if( i1 < 0 ) { i1 = -i1; }
  if( i2 < 0 ) { i2 = -i2; }
  s[0] = sig; s[1] = '\0';
  char int_fill = ' ';
  if( min_sz_int > 1 ) { int_fill = '0'; };
  i2dec( i1, t, min_sz_int,  int_fill ); strncat( s, t, INT_STR_SZ_DEC ); strncat( s, ".", 1 );
  i2dec( i2, t, min_sz_frac, '0'     );  strncat( s, t, INT_STR_SZ_DEC );
  return s;
}



bool arg2long( int narg, int argc, const char * const * argv, long *v,
               long vmin, int vmax )
{
  if( narg >= argc || !argv || !v ) {
    return false;
  }
  char *eptr;
  long l = strtol( argv[narg], &eptr, 0 );
  if( *eptr != 0 ) {
    return false;
  }
  if( l < vmin ) { l = vmin; };
  if( l > vmax ) { l = vmax; };
  *v = l;
  return true;
}

long arg2long_d( int narg, int argc, const char * const * argv, long def,
                 long vmin, long vmax )
{
  long v = def;
  arg2long( narg, argc, argv, &v, vmin, vmax );
  return v;
}

void rev16( uint16_t *v, int n )
{
  for( int i=0; i<n; ++i ) {
    uint16_t t = v[i];
    t = __REV16( t );
    v[i] = t;
  }
}

void bcd_to_char2( uint8_t bcd, char *s )
{
  if( !s ) { return; };
  s[1] = '0' + ( bcd & 0x0F );
  s[0] = '0' + ( bcd >> 4 );
}

void u2_3dig( unsigned n, char *s )
{
  if( !s ) { return; }
  s[2] = '0' + n % 10; n /= 10;
  s[1] = '0' + n % 10; n /= 10;
  s[0] = '0' + n % 10;
}



