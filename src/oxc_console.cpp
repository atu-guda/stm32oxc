#include <oxc_console.h>
#include <oxc_devio.h>


int pr_d( int d )
{
  char b[INT_STR_SZ_DEC];
  return pr( i2dec( d, b ) );
}

int pr_h( uint32_t d )
{
  char b[INT_STR_SZ_HEX];
  return pr( word2hex( d, b ) );
}

int pr_sd( const char *s, int d )
{
  pr( s );
  pr_d( d );
  return pr( NL );
}

int pr_sh( const char *s, int d )
{
  pr( s );
  pr_h( d );
  return pr( NL );
}

