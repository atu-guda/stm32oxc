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

  if( set ) {
    bool ok = true;
    for( int j=idx_s; j<idx_e && ok ; ++j ) {
      ok = set( v, j );
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

  if( get ) {
    return get( idx );
  }

  return false;

}

int NamedFloat::getInt( int idx ) const
{
  return int( getFloat( idx ) );
};


float NamedFloat::getFloat( int idx ) const
{
  float x = 0.0f;
  do_get( x, idx );
  return x;
}


bool NamedFloat::getText( char *d, unsigned maxlen, int idx ) const
{
  const int max_fp_len = 12;
  if( !d || maxlen < max_fp_len ) {
    return false;
  }
  d[0] = '\0';

  if( idx >= -1 ) {
    float x = 0.0f;
    bool ok = do_get( x, idx );
    if( ok ) {
      return ( cvtff( x, d, maxlen ) > 0 );
    } else {
      d[0] = '?'; d[1] = '\0';
      return false;
    }
  }

  if( idx == -2  ||  idx == -3 ) {
    if( maxlen < 12 * ne + 2 ) {
      strcpy( d, "[...]" );
      return false;
    }
    strcat( d, "[ " );
    for( unsigned i=0; i<ne; ++i ) {
      float x = 0.0f;
      do_get( x, idx );
      char tbuf[max_fp_len+4];
      cvtff( x, tbuf, max_fp_len );
      strcat( d, tbuf );
      strcat( d, " " );
    }
    strcat( d, "]" );
    return true;
  }

  return false;
}


bool NamedFloat::out( OutStream &os, int idx ) const
{
  if( idx >= -1 ) {
    float x = 0.0f;
    bool ok = do_get( x, idx );
    if( ok ) {
      os << x;
      return true;
    } else {
      os << '?';
      return false;
    }
  }

  if( idx == -2  ||  idx == -3 ) {
    os << "[ " ;
    for( unsigned i=0; i<ne; ++i ) {
      float x = 0.0f;
      do_get( x, idx );
      os << x << ' ';
    }
    os << " ]";
    return true;
  }

  return false;
}

// -------------------------------------------------------------------

NamedFloats *NamedFloats::global_floats = nullptr;


const NamedFloat* NamedFloats::find( const char *nm, int &idx ) const
{
  if( !nm ) {
    return nullptr;
  }

  char snm[maxSimpleNameLength];
  const char *eptr;
  bool ok = splitNameWithIdx( nm, snm, idx, &eptr );
  if( !ok ) {
    return nullptr;
  }

  auto f = std::find_if( cbegin(), cend(), [snm](auto &p) { return ( strcmp( snm, p.getName() ) == 0 ); } );

  if( f == cend() ) {
    return nullptr;
  }

  if( idx < -3 || idx >= (int)f->size() ) {
    return nullptr;
  }

  return f;
}

bool  NamedFloats::set( const char *nm, float v ) const
{
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }
  if( f->hasFlags( NamedFloat::ro ) ) {
    return false;
  }

  return f->do_set( v, idx );

}

float NamedFloats::get( const char *nm, float def, bool *ok ) const
{
  bool l_ok;
  if( !ok ) {
    ok = &l_ok;
  }
  *ok = false;

  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return def;
  }

  float v;
  *ok = f->do_get( v, idx );
  if( ! *ok ) {
    return def;
  }
  return true;
}


bool NamedFloats::text( const char *nm, char *buf, unsigned bufsz ) const
{
  if( !nm  ||  !buf || bufsz < 12 ) {
    return false;
  }
  bool ok;
  float v = get( nm, 0.0f, &ok ); // TODO: array?
  if( !ok ) {
    return false;
  }
  return ( cvtff( v, buf, bufsz ) > 0 );
}

bool NamedFloats::fromText( const char *nm, const char *s ) const
{
  if( !s || !*s ) {
    return false;
  }
  char *eptr;
  float x = strtof( s, &eptr );
  if( *eptr != '\0' ) {
    return false;
  }
  return set( nm, x );
}

bool NamedFloats::out( OutStream &os, const char *nm ) const
{
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }
  return f->out( os, idx );
}

bool NamedFloats::print( const char *nm ) const
{
  STDOUT_os;
  return out( os, nm );
}

bool NamedFloats::g_print( const char *nm ) // static
{
  if( ! global_floats ) {
    return false;
  }

  return global_floats->print( nm );
}

bool NamedFloats::g_set( const char *nm, float v ) // static
{
  if( ! global_floats ) {
    return false;
  }
  return global_floats->set( nm, v );
}

float NamedFloats::g_get( const char *nm, float def, bool *ok ) // static
{
  if( ! global_floats ) {
    return def;
  }
  return global_floats->get( nm, def, ok );
}

bool  NamedFloats::g_fromText( const char *nm, const char *s ) // static
{
  if( ! global_floats ) {
    return false;
  }
  return global_floats->fromText( nm, s );
}

const char* NamedFloats::g_getName( unsigned i ) // static
{
  if( ! global_floats ) {
    return nullptr;
  }
  return global_floats->getName( i );
}

