#ifndef _OXC_RINGBUF_H
#define _OXC_RINGBUF_H

#include <stdint.h>

#include <oxc_ev.h>

#ifdef USE_OXC
#include <oxc_mutex.h>
#else
#include "oxc_extdef.h"
#endif



class RingBuf {
  public:
   RingBuf( char *a_b, unsigned a_cap ); // used external buf
   #ifndef OXC_NO_OSFUN
   explicit RingBuf( unsigned a_cap );   // dynamic buffer ?
   #endif
   RingBuf( const RingBuf &r ) = delete;
   ~RingBuf();
   RingBuf& operator=( const RingBuf &rhs ) = delete;
   unsigned size() const { return sz; } // w/o block, only info
   unsigned capacity() const { return cap; }
   unsigned isFull() const { return sz == cap; }
   int put( char c ); // blocks, wait
   int tryPut( char c ); // noblocks, fail if busy
   int puts( const char *s ); // blocks, in one lock, returns: number of chars or error
   int puts( const char *s, unsigned l ); // blocks, given length
   int puts_ato( const char *s ); // blocks, all or none
   int puts_ato( const char *s, unsigned l ); // blocks, all or none
   int tryPuts( const char *s ); // try-blocks, as many as possible
   int tryPuts( const char *s, unsigned l ); // try-blocks, given length
   Chst get(); // blocks
   Chst peek(); // blocks
   Chst tryGet(); // noblocks
   Chst tryPeek(); // noblocks
   unsigned gets( char *d, unsigned max_len ); // TODO: move to base class
   unsigned tryGets( char *d, unsigned max_len );
   unsigned tryGetLine( char *d, unsigned max_len ); // only if CR or LF present
   void reset() { MuLock lock( mu ); reset_nolock(); };
   void reset_nolock() { s = e = sz = 0; }
   const char *getBuf() const { return b; }
   unsigned set_n_wait( unsigned n ) { unsigned tmp = n_wait; n_wait = n; return tmp; }
   static void set_wait_fun( void (*vf)(void) ) { wait_fun = vf; }
  protected:
   int  put_nolock( char c ); // false = fail due to full
   Chst get_nolock();
   Chst peek_nolock();
   int puts_nolock( const char *s );
   int puts_nolock( const char *s, unsigned l );
   int gets_nolock( char *d, unsigned max_len );
  protected:
   char *b;                   //* buffer
   const unsigned cap;        //* capacity
   volatile unsigned sz = 0;  //* size
   volatile unsigned s  = 0;  //* start index (over the head)
   volatile unsigned e  = 0;  //* end index
   unsigned n_wait      = 1000; //* number of wait ticks (ms by default)
   Mu_t mu = Mu_init;
   bool was_alloc = false;
   static void (*wait_fun)(void);
};

#endif

