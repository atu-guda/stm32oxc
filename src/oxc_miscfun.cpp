#include <stdlib.h>

#include <oxc_miscfun.h>

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

char* i2dec( int n, char *s )
{
  static char sbuf[INT_STR_SZ_DEC];
  char tbuf[24];
  unsigned u;
  if( !s ) {
    s = sbuf;
  }
  char *bufptr = s, *tmpptr = tbuf + 1;
  *tbuf = '\0';

  if( n < 0 ){ //  sign
    u = ( (unsigned)(-(1+n)) ) + 1; // INT_MIN handling
    *bufptr++ = '-';
  } else {
    u=n;
  }

  do {
    *tmpptr++ = dec_digits[ u % 10 ];
  } while( u /= 10 );

  while( ( *bufptr++ = *--tmpptr ) != '\0' ) /* NOP */;
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
