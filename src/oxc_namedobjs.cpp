// #include <cstdlib>
#include <cstring>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_namedobjs.h>

using namespace std;

// -------------------------------------------------------------------

const NamedObj* NamedObjs::find( const char *nm, int &idx ) const
{
  if( !nm ) {
    return nullptr;
  }

  char snm0[maxSimpleNameLength];
  char snm1[maxExprNameLength];
  const char *eptr;
  bool ok = splitNameWithIdx( nm, snm0, snm1, idx, &eptr );
  if( !ok || *eptr != '\0' ) {
    return nullptr;
  }

  auto f = std::find_if( cbegin(), cend(), [snm0](auto &p) { return ( strcmp( snm0, p->getName() ) == 0 ); } );

  if( f == cend() ) {
    return nullptr;
  }

  auto rf = *f;
  if( idx < -3 || idx >= (int)rf->size() ) {
    return nullptr;
  }

  return rf;
}

bool  NamedObjs::set( const char *nm, int v ) const
{
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }
  if( f->hasFlags( NamedObj::ro ) ) {
    return false;
  }

  return f->set( v, idx );
}

bool  NamedObjs::set( const char *nm, float v ) const
{
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }
  if( f->hasFlags( NamedObj::ro ) ) {
    return false;
  }

  return f->set( v, idx );
}

bool  NamedObjs::set(  const char *nm, const char *s ) const
{
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }
  if( f->hasFlags( NamedObj::ro ) ) {
    return false;
  }

  return f->set( s, idx );
}

bool NamedObjs::get( const char *nm, int &v ) const
{
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }

  return f->get( v, idx );
}

bool NamedObjs::get( const char *nm, float &v ) const
{
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }

  return f->get( v, idx );
}


bool NamedObjs::get( const char *nm, CStr &v ) const
{
  if( !nm  || !v.s || v.n < 2 ) {
    return false;
  }
  v.s[0] = '\0';
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }

  return f->get( v, idx );
}


bool NamedObjs::out( OutStream &os, const char *nm, int fmt ) const
{
  int idx;
  auto f = find( nm, idx );
  if( !f ) {
    return false;
  }
  os << nm << " = ";
  auto ok = f->out( os, idx, fmt );
  os << NL;
  return ok;
}

bool NamedObjs::print( const char *nm, int fmt ) const
{
  STDOUT_os;
  return out( os, nm, fmt );
}


