#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_namedfloats.h>

using namespace std;

bool NamedFloat::do_set( float v, int idx ) const
{
  int idx_s = idx, idx_e = idx + 1;
  if( idx == -2 || idx == -3 ) { // arr[*] ot arr[@]
    idx_s = 0; idx_e = ne;
  } else if ( idx == -1 && ne == 1 ) {  // scalar
    idx_s = 0; idx_e = 1;
  }

  if( idx_s < 0 || idx_e > (int)ne ) {
    return false;
  }

  if( p ) {
    for( int j=idx_s; j<idx_e; ++j ) {
      p[j] = v;
    }
    return true;
  }

  if( fun_set ) {
    bool ok = true;
    for( int j=idx_s; j<idx_e && ok ; ++j ) {
      ok = fun_set( v, j );
    }
    return ok;
  }

  return false;
}

bool NamedFloat::do_get( float &rv, int idx ) const
{
  if( idx == -1 && ne == 1 ) { // scalar;
    idx = 0;
  }

  if( idx < 0 || idx >= (int)(ne) ) { // no ranges here
    return false;
  }

  if( p ) {
    rv = p[idx];
    return true;
  }

  if( fun_get ) {
    rv = fun_get( idx );
    return true;
  }

  return false;

}

bool NamedFloat::get( int &v, int idx ) const
{
  float fv = 0.0f;
  auto ok =  get( fv, idx );
  if( ok ) {
    v = int( fv );
  }
  return ok;
};


bool NamedFloat::get( float &v, int idx ) const
{
  return do_get( v, idx );
}


bool NamedFloat::get( CStr &v, int idx ) const
{
  const int max_fp_len = 12;
  if( !v.s || v.n < max_fp_len ) {
    return false;
  }
  v.s[0] = '\0';

  if( idx >= -1 ) {
    float x = 0.0f;
    bool ok = do_get( x, idx );
    if( ok ) {
      return ( cvtff( x, v.s, v.n ) > 0 );
    } else {
      v.s[0] = '?'; v.s[1] = '\0';
      return false;
    }
  }

  if( idx == -2  ||  idx == -3 ) {
    if( v.n < 12 * ne + 2 ) {
      strcpy( v.s, "[...]" );
      return false;
    }
    strcat( v.s, "[ " );
    for( unsigned i=0; i<ne; ++i ) {
      float x = 0.0f;
      do_get( x, idx );
      char tbuf[max_fp_len+4];
      cvtff( x, tbuf, max_fp_len );
      strcat( v.s, tbuf );
      strcat( v.s, " " );
    }
    strcat( v.s, "]" );
    return true;
  }

  return false;
}

void NamedFloat::do_out( OutStream &os, float x, int fmt )
{
  if( fmt == 1 ) {
    os << FltFmt( x, cvtff_fix );
  }  else if( fmt == 2 ) {
    os << FltFmt( x, cvtff_exp );
  } else {
    os << x;
  }
}


bool NamedFloat::out( OutStream &os, int idx, int fmt ) const
{
  if( idx == -1  &&  ne > 1 ) {
    idx = -2;
  }

  if( idx >= -1 ) {
    float x = 0.0f;
    bool ok = do_get( x, idx );
    if( ok ) {
      do_out( os, x , fmt );
      return true;
    }
    os << '?';
    return false;
  }

  if( idx == -2  ||  idx == -3 ) {
    os << "[ " ;
    for( unsigned i=0; i<ne; ++i ) {
      float x = 0.0f;
      do_get( x, i );
      do_out( os, x , fmt );
      os << ' ';
    }
    os << ']';
    return true;
  }

  return false;
}

bool NamedFloat::set( int v, int idx ) const
{
  return set( (float)(v), idx );
}

bool NamedFloat::set( float v, int idx ) const
{
  return do_set( v, idx );
}

bool NamedFloat::set( const char *v, int idx ) const
{
  if( !v || !*v ) {
    return false;
  }

  char *eptr;
  float x = strtof( v, &eptr );
  if( *eptr != '\0' ) {
    return false;
  }
  return set( x, idx );
}

// -------------------------------------------------------------------

