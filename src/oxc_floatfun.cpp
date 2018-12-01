#include <cstring>

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

OutStream& operator<<( OutStream &os, float rhs ) {
  FloatFmt( rhs ).out( os );
  return os;
}
