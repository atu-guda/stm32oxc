#include <cstdlib>
#include <cctype>
#include <cstring>
#include <algorithm>

#include <oxc_miscfun.h>
// for user_vars
#include <oxc_debug1.h>

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
  if( !s ) {
    return nullptr;
  }
  s[0] = hex_digits[ (uint8_t)(c) >> 4 ];
  s[1] = hex_digits[ c & 0x0F ];
  s[2] = '\0';
  return s;
}

char* word2hex( uint32_t d,  char *s )
{
  if( !s ) {
    return nullptr;
  }
  for( int i=7; i>=0; --i ) {
    s[i] = hex_digits[ d & 0x0F ];
    d >>= 4;
  }
  s[8] = '\0';
  return s;
}

char* u64_2hex( uint64_t d,  char *s )
{
  if( !s ) {
    return nullptr;
  }
  for( int i=15; i>=0; --i ) {
    s[i] = hex_digits[ d & 0x0F ];
    d >>= 4;
  }
  s[16] = '\0';
  return s;
}

char* short2hex( uint16_t d,  char *s )
{
  if( !s ) {
    return nullptr;
  }

  for( int i=3; i>=0; --i ) {
    s[i] = hex_digits[ d & 0x0F ];
    d >>= 4;
  }
  s[4] = '\0';
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
    return nullptr;
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
  i2dec( i1, t, min_sz_int,  int_fill ); strncat( s, t, INT_STR_SZ_DEC ); strncat( s, ".", INT_STR_SZ_DEC );
  i2dec( i2, t, min_sz_frac, '0'     );  strncat( s, t, INT_STR_SZ_DEC );
  return s;
}

__weak int* int_val_ptr( const char *s  )
{
  if( !s || s[0] != '$' || s[2] != '\0' || s[1] < 'a' || s[1] > 'z' ) {
    return nullptr;
  }
  return &( user_vars[ s[1] - 'a'] );
}


bool arg2long( int narg, int argc, const char * const * argv, long *v,
               long vmin, long vmax )
{
  if( narg >= argc || !argv || !v || narg >= argc ) {
    return false;
  }

  const char *s = argv[narg];

  if( int *vx = int_val_ptr( s ) ) {
    *v = *vx;
    return true;
  }

  // special strings: < > %n
  if( s[0] == '=' &&  s[1] == '<' && s[2] == '\0' ) {
    *v = vmin;
    return true;
  }
  if( s[0] == '=' &&  s[1] == '>' && s[2] == '\0' ) {
    *v = vmax;
    return true;
  }

  if( s[0] == '%' && isdigit(s[1]) ) {
    const int percent = std::clamp( atoi(s+1), 0, 100 );
    *v = vmin + ( vmax - vmin ) * percent / 100;
    return true;
  }

  //
  char *eptr;
  const long l = strtol( argv[narg], &eptr, 0 );
  // any non-digital string (like 'def') will get it
  if( *eptr != 0 ) {
    return false;
  }
  *v = clamp( l, vmin, vmax );
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

bool isNameChar1( char c )
{
  if( c >= 'A' && c <= 'Z' ) {
    return true;
  }
  if( c >= 'a' && c <= 'z' ) {
    return true;
  }
  if( c == '_' ) {
    return true;
  }
  return false;
}

bool isNameChar( char c )
{
  if( c >= '0' && c <= '9' ) {
    return true;
  }
  return isNameChar1( c );
}

bool splitNameWithIdx( const char *s, char *d0, char *d1, int &idx, const char **eptr )
{
  const char *ep;
  if( !eptr ) {
    eptr = &ep;
  }
  if( !s || !d0 || !d1 ) {
    *eptr = nullptr;
    return false;
  }

  skip_ws( s );

  d0[0] = d1[0] = '\0';
  char *d = d0;
  unsigned b_len = 0, max_l = maxSimpleNameLength;
  bool first_name = true;
  idx = -1;
  *eptr = s;

  // eat name
  if( ! isNameChar1( *s ) ) {
    return false;
  }
  char last_c = *s++;
  d[b_len++] = last_c;

  for( char c; (c = *s) != '\0' ; ++s, last_c = c ) {
    *eptr = s;
    if( b_len >= max_l-1 ) {
      d0[0] = d1[0] = '\0'; return false;
    }

    if( c == '.' ) {
      if( last_c == '.' ) {
        d0[0] = d1[0] = '\0'; return false;
      }
      if( first_name ) {
        first_name = false;
        d[b_len] = '\0'; d = d1; b_len = 0; max_l = maxExprNameLength;
        continue;
      }
      d[b_len++] = c;
      continue;
    }

    if( ! isNameChar( *s )  ) {
      break;
    }
    d[b_len++] = c;
  }
  d[b_len] = '\0';

  skip_ws( s ); *eptr = s;

  if( *s != '[' ) { // only name
    return true;
  }

  ++s; skip_ws( s );

  if( *s == '*' && *(s+1) == ']' ) {
    idx = -2;
    *eptr = s+2;
    return true;
  }
  if( *s == '@' && *(s+1) == ']' ) {
    idx = -3;
    *eptr = s+2;
    return true;
  }

  // convert decimal index
  const char *epd;
  int v = (int)strtol( s, (char**)( &epd ), 0 );
  if( v < 0 ) {
    *eptr = s;
    return false;
  }

  s = epd;

  skip_ws( s );
  if( *s != ']' ) {
    *eptr = s;
    return false;
  }
  ++s; *eptr = s; idx = v;

  return true;
}

