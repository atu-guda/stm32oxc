#include <cstring>

#include <oxc_ringbuf.h>

using namespace std;

void (*RingBuf::wait_fun)(void) = default_wait1;

RingBuf::RingBuf( char *a_b, unsigned a_cap )
  : b( a_b ), cap( a_cap )
{
}

RingBuf::RingBuf( unsigned a_cap )
  : b( new char[a_cap] ), cap( a_cap ), was_alloc( b != nullptr )
{
}

RingBuf::~RingBuf()
{
  if( was_alloc ) {
    delete[] b; b = nullptr; s = e = sz = 0;
  }
}

int RingBuf::put_nolock( char c )
{
  if( sz >= cap ) {
    return 0;
  }
  unsigned sn = s + 1;
  if( sn >= cap ) {
    sn = 0;
  }
  b[s] = c; s = sn;  ++sz;

  for( int x=0; x<was_get_ISR; ++x ) { // fake get
    (void)get_nolock();
  }
  was_get_ISR = 0;
  return 1;
}

int RingBuf::put( char c )
{
  for( unsigned i=0; i<n_wait; ++i ) {
    {
      MuTryLock lock( mu );
      if( lock.wasAcq() ) {
        if( put_nolock( c ) > 0 ) {
          return 1;
        }
      }
    }
    wait_fun();
  }
  return -1;
}

int RingBuf::tryPut( char c )
{
  MuTryLock lock( mu );
  if( lock.wasAcq() ) {
    return put_nolock( c );
  }
  return -1;
}


int RingBuf::puts_nolock( const char *s )
{
  unsigned r;
  for( r=0; *s && put_nolock( *s ) ; ++s, ++r ) {
  }
  return r;
}

int RingBuf::puts_nolock( const char *s, unsigned l )
{
  int r;
  for( r=0; (unsigned)r<l && put_nolock( *s ) ; ++s, ++r ) {
  }
  return r;
}


int RingBuf::puts( const char *s )
{
  if( !s ) {
    return 0;
  }
  unsigned l = strlen( s );
  return puts( s, l );
}

int RingBuf::puts( const char *s, unsigned l )
{
  int w = 0;
  unsigned np;
  for( np=0; np<l; np += w ) {
    unsigned to_put = l - np;
    unsigned l_cur = (to_put > cap ) ? cap : to_put;
    w = puts_ato( s + np, l_cur );
    if( w < 1 ) {
      return np;
    }
  }
  return np;
}


int RingBuf::puts_ato( const char *s )
{
  if( !s ) {
    return 0;
  }
  unsigned l = strlen( s );
  return puts_ato( s, l );
}

int RingBuf::puts_ato( const char *s, unsigned l )
{
  if( !s || l < 1 ) {
    return 0;
  }
  if( l > cap ) {
    return -5;
  }

  for( unsigned nw = 0; nw < n_wait; ++nw ) {
    {
      MuTryLock lock( mu );
      if( !lock.wasAcq() ) {
        continue;
      }
      auto n_free = cap - sz;
      if( l <= n_free ) {
        for( unsigned r=0; r<l; ++s, ++r ) {
          put_nolock( *s );
        }
        // for( int x=0; x<was_get_ISR; ++x ) { // fake get
        //   (void)get_nolock();
        // }
        // was_get_ISR = 0;
        return l;
      }
    }
    wait_fun();
    continue;
  }
  return -4;
}

int RingBuf::tryPuts( const char *s )
{
  MuTryLock lock( mu );
  if( ! lock.wasAcq() ) {
    return -1;
  }
  return puts_nolock( s );
}

int RingBuf::tryPuts( const char *s, unsigned l )
{
  MuTryLock lock( mu );
  if( ! lock.wasAcq() ) {
    return -1;
  }
  return puts_nolock( s, l );
}


Chst RingBuf::get_nolock()
{
  if( sz < 1 ) {
    return Chst( '\0', Chst::st_empty );
  }
  char c = b[e++];
  if( e >= cap ) {
    e = 0;
  }
  --sz;
  return c;
}


Chst RingBuf::get()
{
  MuLock lock( mu );
  return get_nolock();
}

Chst RingBuf::tryGet()
{
  MuTryLock lock( mu );
  if( lock.wasAcq() ) {
    return get_nolock();
  }
  return Chst( '\0', Chst::st_lock );
}

Chst RingBuf::getFromISR()
{
  auto r = tryGet();
  if( ! r.locked() ) {
    return r;
  }
  if( sz == 0 ) {
    return Chst( '\0', Chst::st_empty );
  }
  ++was_get_ISR;
  return b[e];
}

int RingBuf::gets_nolock( char *d, unsigned max_len )
{
  if( !s  || max_len < 1 ) {
    return 0;
  }
  int i;
  for( i=0; (unsigned)i<max_len; ++i ) {
    auto x = get_nolock();
    if( !x.good() ) {
      break;
    }
    *d++ = x.c;
  }
  return i;
}

unsigned RingBuf::gets( char *d, unsigned max_len )
{
  if( !s  || max_len < 1 ) {
    return 0;
  }
  MuLock lock( mu );
  return gets_nolock( d, max_len );
}

unsigned RingBuf::tryGets( char *d, unsigned max_len )
{
  if( !s  || max_len < 1 ) {
    return 0;
  }
  MuTryLock lock( mu );
  if( lock.wasAcq() ) {
    return gets_nolock( d, max_len );
  }
  return 0;
}

