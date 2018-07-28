#include <oxc_ringbuf.h>

using namespace std;

#ifndef USE_OXC
#include <pthread.h>
using Mu_t  = pthread_mutex_t;
int pthread_mutex_waitlock( pthread_mutex_t *mutex, unsigned ms  )
{
  for( unsigned i=0; i<ms; ++i ) {
    if( pthread_mutex_trylock( mutex ) ) {
        return 1;
    }
  }
  return 0;
}
#endif


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

bool RingBuf::put_nolock( char c )
{
  if( sz >= cap ) {
    return false;
  }
  unsigned sn = s + 1;
  if( sn >= cap ) {
    sn = 0;
  }
  b[s] = c; s = sn;  ++sz;
  return true;
}

bool RingBuf::put( char c )
{
  MuLock lock( mu );
  return put_nolock( c );
}

bool RingBuf::tryPut( char c )
{
  MuTryLock lock( mu );
  if( lock.wasAcq() ) {
    return put_nolock( c );
  }
  return false;
}

bool RingBuf::waitPut( char c, uint32_t ms  )
{
  MuWaitLock lock( mu, ms );
  if( lock.wasAcq() ) {
    return put_nolock( c );
  }
  return false;
}

unsigned RingBuf::puts_nolock( const char *s )
{
  unsigned r;
  for( r=0; *s && put_nolock( *s ) ; ++s, ++r ) {
  }
  return r;
}

unsigned RingBuf::puts_nolock( const char *s, unsigned l )
{
  unsigned r;
  for( r=0; r<l && put_nolock( *s ) ; ++s, ++r ) {
  }
  return r;
}


unsigned RingBuf::puts( const char *s )
{
  MuLock lock( mu );
  return puts_nolock( s );
}

unsigned RingBuf::puts( const char *s, unsigned l )
{
  MuLock lock( mu );
  return puts_nolock( s, l );
}

unsigned RingBuf::tryPuts( const char *s )
{
  MuTryLock lock( mu );
  if( ! lock.wasAcq() ) {
    return 0;
  }
  return puts_nolock( s );
}

unsigned RingBuf::tryPuts( const char *s, unsigned l )
{
  MuTryLock lock( mu );
  if( ! lock.wasAcq() ) {
    return 0;
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

Chst RingBuf::waitGet( uint32_t ms )
{
  MuWaitLock lock( mu, ms );
  if( lock.wasAcq() ) {
    return get_nolock();
  }
  return Chst( '\0', Chst::st_lock );
}

unsigned RingBuf::gets_nolock( char *d, unsigned max_len )
{
  if( !s  || max_len < 1 ) {
    return 0;
  }
  unsigned i;
  for( i=0; i<max_len; ++i ) {
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
  return gets_nolock( d, max_len );
}
