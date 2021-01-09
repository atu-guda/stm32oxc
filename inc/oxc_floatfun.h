#ifndef _OXC_FLOATFUN_H
#define _OXC_FLOATFUN_H

#include <float.h>

#include <oxc_miscfun.h>
#include <oxc_outstream.h>

// check, if we have cheap double
#if defined(OXC_FORCE_DOUBLE) || defined(__x86_64) || defined(__i386) || ( defined(__ARM_FP) && (__ARM_FP & 8 ) )
  #define OXC_HAVE_DOUBLE 1
  #define xfloat double
#else
  #define xfloat float
#endif

#ifdef OXC_HAVE_DOUBLE
  #define OXC_NEED_DOUBLE_OUT
#endif

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

float str2float_d( const char *s, float def, float vmin = -__FLT_MAX__, float vmax = __FLT_MAX__ );
float arg2float_d( int narg, int argc, const char * const * argv, float def,
                 float vmin = -__FLT_MAX__, float vmax = __FLT_MAX__ );
// TODO: callback for parameter parsing

const unsigned buf_len_float  = 36;
const unsigned buf_len_double = 72;
// const char* const def_float_fmt_init = "%#g";

class FltFmt : public OutStreamFmt {
  public:
   explicit FltFmt( float a, uint32_t a_flg = cvtff_auto, int a_w = 0, int a_prec = 99 ) :
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

OutStream& operator<<( OutStream &os, float rhs );

#ifdef OXC_NEED_DOUBLE_OUT
extern double exp10id( int x );

constexpr inline  int double_default_width { 15 };

int cvtfd( double f, char *buf, unsigned bufsz, uint32_t flg = cvtff_auto, int w = double_default_width, int prec = 99 );

double str2double_d( const char *s, double def, double vmin = -__DBL_MAX__, double vmax = __DBL_MAX__ );
double arg2double_d( int narg, int argc, const char * const * argv, double def,
                 double vmin = -__DBL_MAX__, double vmax = __DBL_MAX__ );
// TODO: callback for parameter parsing


class DblFmt : public OutStreamFmt {
  public:
   explicit DblFmt( double a, uint32_t a_flg = cvtff_auto, int a_w = 0, int a_prec = 99 ) :
     v( a ), flg( a_flg ),
     w( ( a_w > 1 ) ? a_w : auto_width ),
     prec( a_prec ) {};
   operator double() const { return v; }
   const double v;
   virtual void out( OutStream &os ) const override;
   static int set_auto_width( int aw ) { auto t = auto_width; auto_width = aw; return t; }
  protected:
   uint32_t flg;
   int w, prec;
   static int auto_width;
};

OutStream& operator<<( OutStream &os, double rhs );

#endif

#ifdef OXC_HAVE_DOUBLE

#define XFmt DblFmt
#define xfloat_default_width double_default_width
#define cvtfx cvtfd
#define str2xfloat_d str2double_d
#define arg2xfloat_d arg2double_d
#define XFLOAT_MAX     __DBL_MAX__
#define XFLOAT_NMAX   (-__DBL_MAX__)
#define XFLOAT_LARGE  (__DBL_MAX__/10)
#define XFLOAT_NLARGE (-__DBL_MAX__/10)
#define XFLOAT_MIN __DBL_MIN__
#define XFLOAT_EPS __DBL_EPSILON__
#define XFLOAT_C(c) c

// -------------------- without double -----------------------------------
#else

#define xfloat_default_width float_default_width
#define XFmt FltFmt
#define cvtfx cvtff
#define str2xfloat_d str2float_d
#define arg2xfloat_d arg2float_d
#define XFLOAT_MAX     __FLT_MAX__
#define XFLOAT_NMAX   (-__FLT_MAX__)
#define XFLOAT_LARGE  (__FLT_MAX__/10)
#define XFLOAT_NLARGE (-__FLT_MAX__/10)
#define XFLOAT_MIN __FLT_MIN__
#define XFLOAT_EPS __FLT_EPSILON__
#define XFLOAT_C(c) c ## f

#endif


#endif
