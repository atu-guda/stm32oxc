#include <cstring>
#include <stdlib.h>

#include <oxc_floatfun.h>

using namespace std;


char FloatFmt::def_fmt[buf_len_fmt] = "%#g";

void FloatFmt::set_def_fmt( const char *fmt )
{
  if( !fmt || ! *fmt ) { // reset to default if empty string
    strcpy( def_fmt, def_float_fmt_init );
    return;
  }
  def_fmt[0] = '\0';
  strncat( def_fmt, fmt, sizeof(def_fmt)-1 );
}

void FloatFmt::out( OutStream &os ) const
{
  char buf[buf_len_float];
  snprintf( buf, sizeof(buf), f ? f : def_fmt, (double)v );
  os << buf;
}

OutStream& operator<<( OutStream &os, double rhs ) {
  FloatFmt( rhs ).out( os );
  return os;
}

OutStream& operator<<( OutStream &os, float rhs ) {
  return os << (double)(rhs);
};

float str2float_d( const char *s, float def, float vmin, float vmax )
{
  float v = def;
  char *eptr;
  if( s && *s ) {
    // TODO: callback
    float t = strtof( s, &eptr );
    if( *eptr == '\0' ) {
      v = t;
    }
  }
  if( v < vmin ) { v = vmin; };
  if( v > vmax ) { v = vmax; };
  return v;
}

float arg2float_d( int narg, int argc, const char * const * argv, float def, float vmin, float vmax )
{
  float v = def;
  if(  narg < argc && argv != nullptr ) {
    v = str2float_d( argv[narg], def, vmin, vmax );
  }
  return v;
}

