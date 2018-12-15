#ifndef _OXC_FLOATFUN_H
#define _OXC_FLOATFUN_H

#include <float.h>

#include <oxc_miscfun.h>
#include <oxc_outstream.h>

const unsigned buf_len_float = 64;
const unsigned buf_len_fmt   = 32;
const char* const def_float_fmt_init = "%#g";

//* helper classes for stream output for float
class FloatFmt : public OutStreamFmt {
  public:
   explicit FloatFmt( float a, const char *fmt = nullptr ) : v(a), f( fmt ) {};
   operator float() const { return v; }
   const float v;
   const char *f;
   static const char* get_def_fmt() { return def_fmt; };
   static void set_def_fmt( const char *fmt );
   virtual void out( OutStream &os ) const override;
  protected:
   static char def_fmt[buf_len_fmt];
};

OutStream& operator<<( OutStream &os, float rhs );
OutStream& operator<<( OutStream &os, double rhs );

float str2float_d( const char *s, float def, float vmin = -3.402e+38F, float vmax = 3.402e+38F );
float arg2float_d( int narg, int argc, const char * const * argv, float def,
                 float vmin = -3.402e+38F, float vmax = 3.402e+38F );
// TODO: callback for parameter parsing

#endif
