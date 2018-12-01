#ifndef _OXC_FLOATFUN_H
#define _OXC_FLOATFUN_H

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

#endif
