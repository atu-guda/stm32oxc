#ifndef _OXC_GCODE_H
#define _OXC_GCODE_H

#include <cmath>

#include <oxc_auto.h>
#include <oxc_floatfun.h>

class GcodeBlock;

class GcodeBlock {
  public:
    enum class GcodeState {
      init = 0, // initial state
      param,    // after letter X 1.23
      quoted,   // inside " " in param
      comment,  // between ( ), ; + to_end
      error     // + to_end?
    };
    enum RcCode { // TODO: ReturnCore
      rcOk     =  0,
      rcWarn   =  1,
      rcUnsupp =  2,
      rcEnd    = 10,
      rcErr    = 11,
      rcFatal  = 12,
    };
    enum ErrCode {
      errNone     =  0,
      errComment  =  1,
      errString   =  2,
      errChar     =  3,
      errValue    =  4,
      errMach     = 10
    };
    using ActFun = int(*)( const GcodeBlock &gc );
    static const unsigned n_p { 'Z'-'A' + 3 }; // 28, A-Z, end, ?
    static constexpr const char *const axis_chars = "XYZEVUW?"; // TODO: no?
                                                                //
    explicit GcodeBlock( ActFun a_f ) : act_fun( a_f ) { init(); } ;
    void init();
    void sub_init();
    int process( const char *s );
    xfloat fpv( char c ) const { return fp[ c - 'A' ]; };
    int    ipv( char c ) const { return int(fp[ c - 'A' ]); };
    bool is_set( char c ) const { return std::isfinite( fp[c-'A'] ); };
    xfloat fpv_or_def( char c, xfloat def ) const { unsigned i=c-'A'; return std::isfinite(fp[i]) ? fp[i] : def ; };
    int    ipv_or_def( char c, int    def ) const { return int( fpv_or_def(c,def) ); };
    const xfloat* date() const { return fp; } // beware: NAN for uninited
    void dump() const;
    int get_err_pos()  const { return err_pos; };
    int get_err_code() const { return err_code; }
    int get_mach_rc()  const { return mach_rc; }
    const char* get_str0()  const { return str0; }
    const char* get_str1()  const { return str1; }

  private:
    static const unsigned max_str_sz { 80 };
    static const unsigned max_gm_funcs { 200 };
    ActFun act_fun;
    xfloat fp[n_p];              // real params
    char str0[max_str_sz+2], str1[max_str_sz+2];
    int err_pos  { 0 };
    int err_code { 0 };
    int mach_rc  { 0 };
};



#define COMMON_GM_CODE_CHECK if( !cb || !ms ) {  return GcodeBlock::rcFatal;  }

#endif
