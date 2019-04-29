#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>

using namespace std;

bool NamedInt::do_set( int v, int idx ) const
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

bool NamedInt::do_get( int &rv, int idx ) const
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

bool NamedInt::get( int &v, int idx ) const
{
  return do_get( v, idx );
};


bool NamedInt::get( float &v, int idx ) const
{
  int iv = 0;
  auto ok =  get( iv, idx );
  if( ok ) {
    v = float( iv );
  }
  return ok;
}


bool NamedInt::get( CStr &v, int idx ) const
{
  if( !v.s || v.n < INT_STR_SZ_DEC ) {
    return false;
  }
  v.s[0] = '\0';

  if( idx >= -1 ) {
    int x = 0;
    bool ok = do_get( x, idx );
    if( ok ) {
      i2dec_n( x, v.s, v.n );
      return true;
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
      int x = 0;
      do_get( x, idx );
      char tbuf[INT_STR_SZ_DEC+4];
      i2dec_n( x, tbuf, INT_STR_SZ_DEC );
      strcat( v.s, tbuf );
      strcat( v.s, " " );
    }
    strcat( v.s, "]" );
    return true;
  }

  return false;
}


bool NamedInt::out( OutStream &os, int idx ) const
{
  if( idx == -1  &&  ne > 1 ) {
    idx = -2;
  }

  if( idx >= -1 ) {
    int x = 0;
    bool ok = do_get( x, idx );
    if( ok ) {
      os << x;
      return true;
    }
    os << '?';
    return false;
  }

  if( idx == -2  ||  idx == -3 ) {
    os << "[ " ;
    for( unsigned i=0; i<ne; ++i ) {
      int x = 0;
      do_get( x, i );
      os << x << ' ';
    }
    os << " ]";
    return true;
  }

  return false;
}

bool NamedInt::set( int v, int idx ) const
{
  return do_set( v, idx );
}

bool NamedInt::set( float v, int idx ) const
{
  return set( (int)(v), idx );
}

bool NamedInt::set( const char *v, int idx ) const
{
  if( !v || !*v ) {
    return false;
  }

  char *eptr;
  int x = 0;
  if( v[0] == '-' ) {
    x = strtol( v, &eptr, 0 );
  } else {
    x = strtoul( v, &eptr, 0 );
  }
  if( *eptr != '\0' ) {
    return false;
  }
  return set( x, idx );
}

// -------------------------------------------------------------------

