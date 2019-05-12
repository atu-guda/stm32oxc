// #include <cstdlib>
#include <cstring>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_namedobjs.h>

using namespace std;

// -------------------------------------------------------------------

const NamedObjs*  NamedObj::getSubObjs() const
{
  return nullptr;
}

// -------------------------------------------------------------------

bool NamedSubObj::get( int & /*v*/, int /*idx*/ ) const
{
  return false;
};


bool NamedSubObj::get( float & /*v*/, int /*idx*/ ) const
{
  return false;
}


bool NamedSubObj::get( CStr & /*v*/, int /*idx*/ ) const
{
  return false;
}


bool NamedSubObj::out( OutStream &os, int idx, int fmt ) const
{
  os << "{" NL;
  bool ok = no->out( os, "", fmt );
  os << "}";
  return ok;
}

bool NamedSubObj::set( int /*v*/, int /*idx*/ ) const
{
  return false;
}

bool NamedSubObj::set( float /*v*/, int /*idx*/ ) const
{
  return false;
}

bool NamedSubObj::set( const char * /*v*/, int /*idx*/ ) const
{
  return false;
}


const NamedObjs*  NamedSubObj::getSubObjs() const
{
  return no;
}

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

  if( idx < -3 || idx >= (int)( (*f)->size() ) ) {
    return nullptr;
  }

  if( snm1[0] != '\0' ) { // have subobjects in nm
    auto sub = (*f)->getSubObjs();
    if( !sub ) {
      return nullptr;
    }
    return sub->find( snm1, idx );
  }

  return *f;
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
  if( nm && *nm && ! ( nm[0] == '*' && nm[1] == '\0' ) ) { // name is given
    auto f = find( nm, idx );
    if( !f ) {
      os << "# Error: name not found \"" << nm << "\"" NL;
      return false;
    }
    os << nm << " = ";
    auto ok = f->out( os, idx, fmt );
    os << NL;
    return ok;
  }

  // all names
  for( auto f : *this ) {
    os << f->getName() << " = ";
    f->out( os, -1, fmt );
    os << NL;
  }
  return true;

}

bool NamedObjs::print( const char *nm, int fmt ) const
{
  STDOUT_os;
  return out( os, nm, fmt );
}


