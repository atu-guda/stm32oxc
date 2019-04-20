#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_namedfloats.h>

#include <cstdlib>
#include <cstring>

using namespace std;


NamedFloats *NamedFloats::global_floats = nullptr;

const NamedFloat* NamedFloats::find( const char *nm ) const
{
  if( !nm ) {
    return nullptr;
  }
  // TODO: check and use, may be with std::find
  // for( const auto &f : *this ) {
  //   if( strcmp( nm, f.name ) == 0 ) {
  //     return &f;
  //   }
  // }

  for( const auto *f = fl; f->name != nullptr; ++f ) {
    if( strcmp( nm, f->name ) == 0 ) {
      return f;
    }
  }
  return nullptr;
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

