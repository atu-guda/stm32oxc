#include <cstring>
#include <cmath>

#include <oxc_floatfun.h>

using namespace std;

float exp10if( int x )
{
  float r = 1.0f;
  float k = 10.0f;
  if( x < 0 ) {
    k = 0.1f;
    x = -x;
  }

  for( int i=0; i<x; ++i ) {
    r *= k;
  }
  return r;
}

int cvtff( float f, char *buf, unsigned bufsz, uint32_t flg, int w, int prec )
{
  if( !buf || bufsz < 6 || bufsz < (unsigned)(w+1) ) {
    return 0;
  }
  if(  w < 3 ) {
    buf[0] = '\0';
    return 0;
  }

  int sign = signbit( f );
  if( sign ) {
    f = -f; sign = -1;  buf[0] = '-';
  } else {
    sign = 1;  buf[0] = ( flg & cvtff_sign ) ? '+' : ' ';
  }
  buf[w] = '\0';

  if( f < 9.99999e-35f ) { // BUG: bad format. TODO: check
    f = 0.0f;
  }

  int exp1 = 0;
  if( f > __FLT_DENORM_MIN__ ) {
    exp1 = floor( log10f( f ) );
  }

  bool need_exp = false;

  if( exp1 > (w-2) ) { // overflow for fixed representation
    // cerr << "## fix overflow exp1= " << exp1 << " w= " << w << endl;
    if( ( flg & cvtff_force_fix_high ) /* || w < 7 */ ) {
      memset( buf+1, '#', w-1 ); return w;
    }
    need_exp = true;
  }

  int n_dig_exp = w - 6; // "+e+00"
  int n_dig_fix = w - 2 + ( (exp1<0) ? exp1 : 0 );

  if( n_dig_exp > n_dig_fix ) {
    if( ! ( flg & cvtff_force_fix_low ) ) {
      need_exp = true;
    }
  }

  bool do_exp = ( flg & cvtff_exp ) || ( need_exp );

  // cerr << "# \"" << setw(w) << f << "\" n_dig_exp= " << n_dig_exp << " n_dig_fix= " << n_dig_fix << " do_exp= " << do_exp << " exp1= " << exp1 <<  " w= " << w <<endl;

  if( do_exp ) {
    if( w < 7 ) {
      memset( buf+1, '#', w-1 ); return w;
    }
    char *p = buf + 1;
    // float fn = f * exp10( -exp1 );
    long fnx = lroundf( f * exp10if( n_dig_exp - 1 - exp1 ) );
    float fn = fnx / exp10if( n_dig_exp - 1 );
    // cerr << "# float: fn= " << setw(16) << fn << " fnx= " << fnx << " fnR= " << setw(16) << fn << endl;
    int pwr = 1;

    for( int pos = w - 6; pos >= 0 ; --pos ) {
      int f0 = int(fn);
      *p++ = (char)( f0 + '0' );
      --pwr;
      if( pwr == 0 ) {
        *p++ = '.'; --pos;
      }
      fn -= f0;
      fn *= 10;
    }
    *p++ = 'e';
    *p++ = ( exp1 >= 0 ) ? '+' : '-';
    int exp2 = abs( exp1 );
    *p++ = char( ( exp2 / 10 ) + '0' );
    *p++ = char( ( exp2 % 10 ) + '0' );
    *p = '\0';
    return w;
  }

  // fixed point

  int prec_max = w - 2 - ( exp1+1 );
  if( prec_max > w - 3 ) {
    prec_max = w - 3;
  }
  if( prec > prec_max ) {
    prec = prec_max;
  }
  bool show_dpoint = true;
  if( prec < 0 ) { // case: need to eat decimal point
    prec = 0;
    show_dpoint = false;
  }
  float fni = f * exp10if( prec );
  unsigned long long fn = llroundf( sign * fni ) * sign;
  // cerr << "# fix: prec = " << prec << " fn= " << fn << " fni= " << setprecision( __FLT32_DECIMAL_DIG__ ) << fni << " sign= " << sign;

  int dpos = prec;
  for( int i=w-1; i>0; --i ) {
    if( dpos == 0 && show_dpoint ) {
      buf[i--] = '.';
    }
    --dpos;
    buf[i] = (char)( '0' + (fn % 10) );
    fn /= 10;
  }
  // cerr  << endl;

  return w;
}

// -------------------------------------------------------------- 

int FltFmt::auto_width = float_default_width;

void FltFmt::out( OutStream &os ) const
{
  char buf[buf_len_float];
  if( cvtff( v, buf, sizeof(buf), flg, w, prec ) > 0 ) {
    os << buf;
  }
}

OutStream& operator<<( OutStream &os, float rhs ) {
   FltFmt( rhs ).out( os );
   return os;
};


// -------------------------------------------------------------- 

//
// char FloatFmt::def_fmt[buf_len_fmt] = "%#g";
//
// void FloatFmt::set_def_fmt( const char *fmt )
// {
//   if( !fmt || ! *fmt ) { // reset to default if empty string
//     strcpy( def_fmt, def_float_fmt_init );
//     return;
//   }
//   def_fmt[0] = '\0';
//   strncat( def_fmt, fmt, sizeof(def_fmt)-1 );
// }
//
// void FloatFmt::out( OutStream &os ) const
// {
//   char buf[buf_len_float];
//   snprintf( buf, sizeof(buf), f ? f : def_fmt, (double)v );
//   os << buf;
// }

// OutStream& operator<<( OutStream &os, double rhs ) {
//   FloatFmt( rhs ).out( os );
//   return os;
// }
//
// OutStream& operator<<( OutStream &os, float rhs ) {
//    FloatFmt( (double)rhs ).out( os );
//    return os;
// };

// -------------------------------------------------------------- 



// -------------------------------------------------------------- 


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

