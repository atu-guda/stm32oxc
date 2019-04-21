#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_namedfloats.h>

using namespace std;


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

  auto f = std::find_if( cbegin(), cend(), [snm](auto &p) { return ( strcmp( snm, p.name ) == 0 ); } );

  if( f == cend() ) {
    return nullptr;
  }

  if( idx < -3 || idx >= (int)f->ne ) {
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
  if( f->flags & NamedFloat::ro ) {
    return false;
  }

  if( f->set != nullptr ) { // TODO: idx
    return f->set( v );
  }

  if( f->p == nullptr ) {
    return false;
  }

  if( idx == -2 || idx == -3 ) {
    for( unsigned j=0; j<f->ne; ++j ) {
      (f->p)[j] = v;
    }
    return true;
  }

  if( idx < 0 ) {
    if( f->ne != 1 ) { // for arrays index required
      return false;
    }
    idx = 0;
  }

  (f->p)[idx] = v;
  return true;
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

  if( f->get != nullptr ) { // TODO: idx
    *ok = true;
    return f->get();
  }

  if( f->p == nullptr ) {
    return def;
  }

  if( idx == -1 ) {
    if( f->ne == 1 ) { // non-array
      idx = 0;
    } else {
      return def;
    }
  }

  if( idx < 0 || (unsigned)idx >= f->ne ) {
    return def;
  }

  *ok = true;
  return (f->p)[idx];
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

bool NamedFloats::print( const char *nm ) const
{
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }
  STDOUT_os;

  if( f->ne == 1 && idx == -1 ) { // single var
    idx = 0;
  }

  os << nm << " = ";

  // single
  if( idx >= 0 ) {
    float x = 0.0f;
    if( f->get != nullptr ) {
      x = f->get();
    } else if( f->p != nullptr ) {
      x = (f->p)[idx];
    }
    os << x << NL;
    return true;
  }

  if( f->get != nullptr ) {
    os << "# Error: not work for now" NL;
    return false;
  }

  os << "[";
  for( unsigned j=0; j<f->ne; ++j ) {
    os << ' ' << (f->p)[j];
  }
  os << " ]" NL;

  return true;
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

