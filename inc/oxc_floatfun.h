#ifndef _OXC_FLOATFUN_H
#define _OXC_FLOATFUN_H

#include <float.h>

#include <oxc_miscfun.h>
#include <oxc_outstream.h>

enum cvtff_flags {
  cvtff_auto = 0,
  cvtff_exp = 2,
  cvtff_sign = 0x10,
  cvtff_fill0 = 0x20,
  cvtff_force_fix_low  = 0x40, // output is near to zero or zero
  cvtff_force_fix_high = 0x80, // output is "####..." to show overflow
  cvtff_force_fix = cvtff_force_fix_low | cvtff_force_fix_high,
  cvtff_fix = cvtff_force_fix
};

extern float exp10if( int x );

constexpr inline  int float_default_width { 11 };

int cvtff( float f, char *buf, unsigned bufsz, uint32_t flg = cvtff_auto, int w = float_default_width, int prec = 99 );

const unsigned buf_len_float = 36;
// const unsigned buf_len_float = 72;
// const unsigned buf_len_fmt   = 32;
// const char* const def_float_fmt_init = "%#g";

class FltFmt : public OutStreamFmt {
  public:
   explicit FltFmt( float a, uint32_t a_flg = cvtff_auto, int a_w = float_default_width, int a_prec = 99 ) :
     v( a ), flg( a_flg ),
     w( ( a_w > 1 ) ? a_w : auto_width ),
     prec( a_prec ) {};
   operator float() const { return v; }
   const float v;
   virtual void out( OutStream &os ) const override;
   static int set_auto_width( int aw ) { auto t = auto_width; auto_width = aw; return t; }
  protected:
   uint32_t flg;
   int w, prec;
   static int auto_width;
};

//* helper classes for stream output for floating point:
// used double due to auto promotion float-double in ...
// class FloatFmt : public OutStreamFmt {
//   public:
//    explicit FloatFmt( double a, const char *fmt = nullptr ) : v( a ), f( fmt ) {};
//    operator double() const { return v; }
//    const double v;
//    const char *f;
//    static const char* get_def_fmt() { return def_fmt; };
//    static void set_def_fmt( const char *fmt );
//    virtual void out( OutStream &os ) const override;
//   protected:
//    static char def_fmt[buf_len_fmt];
// };

// OutStream& operator<<( OutStream &os, double rhs );
OutStream& operator<<( OutStream &os, float rhs );

float str2float_d( const char *s, float def, float vmin = -3.402e+38F, float vmax = 3.402e+38F );
float arg2float_d( int narg, int argc, const char * const * argv, float def,
                 float vmin = -3.402e+38F, float vmax = 3.402e+38F );
// TODO: callback for parameter parsing

#endif
