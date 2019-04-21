#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_namedfloats.h>

using namespace std;


NamedFloats *NamedFloats::global_floats = nullptr;

const NamedFloat* NamedFloats::find( const char *nm ) const
{
  if( !nm ) {
    return nullptr;
  }

  auto f = std::find_if( cbegin(), cend(), [nm](auto &p) { return ( strcmp( nm, p.name ) == 0 ); } );

  return ( f != cend() ) ? f : nullptr;
}

bool  NamedFloats::set( const char *nm, float v ) const
{
  auto f = find( nm );
  if( !f ) {
    return false;
  }
  if( f->flags & NamedFloat::flg_ro ) {
    return false;
  }
  if( f->set != nullptr ) {
    return f->set( v );
  }
  if( f->p != nullptr ) {
    *(f->p) = v;
    return true;
  }
  return false;
}

float NamedFloats::get( const char *nm, float def, bool *ok ) const
{
  bool l_ok;
  if( !ok ) {
    ok = &l_ok;
  }
  auto f = find( nm );
  if( !f ) {
    *ok = false;
    return def;
  }

  *ok = true;
  if( f->get != nullptr ) {
    return f->get();
  }
  if( f->p != nullptr ) {
    return *(f->p);
  }
  *ok = false;
  return def;
}


bool NamedFloats::text( const char *nm, char *buf, unsigned bufsz ) const
{
  if( !nm  ||  !buf || bufsz < 12 ) {
    return false;
  }
  bool ok;
  float v = get( nm, 0.0f, &ok );
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

bool NamedFloats::g_print( const char *nm ) // static
{
  if( ! global_floats ) {
    return false;
  }
  bool ok;
  float x = global_floats->get( nm, 0.0f, &ok );
  if( !ok ) {
    return false;
  }
  STDOUT_os;
  os << nm << " = " << x << NL;
  return true;
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

bool  NamedFloats::g_fromText( const char *nm, const char *s ) //static
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

