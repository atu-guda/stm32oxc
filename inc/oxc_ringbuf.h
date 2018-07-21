#ifndef _OXC_RINGBUF_H
#define _OXC_RINGBUF_H

#ifdef USE_OXC
#include <oxc_base.h>
#endif

// TODO: classes for Mu_t handling: move to oxc_main or oxc_mu

class Chst { // char + status
  public:
   enum {
     st_good = 0, st_full = 1, st_empty = 2, st_lock = 4
   };
   Chst( char ch ) : c( ch ), st( st_good ) {};
   Chst( char ch, uint8_t a_st ) : c( ch ), st( a_st ) {};
   char c;
   uint8_t st;
   bool good()   const noexcept { return st == st_good;  }
   bool full()   const noexcept { return st == st_full;  }
   bool empty()  const noexcept { return st == st_empty; }
   bool locked() const noexcept { return st == st_lock;  }
};
static_assert( sizeof(Chst) == 2 );

class RingBuf {
  public:
   RingBuf( char *a_b, unsigned a_cap ); // used external buf
   // may be with dynamic buffer ?
   RingBuf( const RingBuf &r ) = delete;
   RingBuf& operator=( const RingBuf &rhs ) = delete;
   unsigned size() const { return sz; } // w/o block?
   unsigned capacity() const { return cap; }
   unsigned isFull() const { return sz == cap; }
   bool put( char c ); // blocks, wait
   bool tryPut( char c ); // noblocks, fail if busy
   bool waitPut( char c, uint32_t ms = 100 ); // wait + try, delay_ms(1)
   unsigned puts( const char *s ); // blocks, in one lock, ret: number of char
   unsigned puts( const char *s, unsigned l ); // blocks, given length
   unsigned tryPuts( const char *s ); // try-blocks
   unsigned tryPuts( const char *s, unsigned l ); // try-blocks, given length
   Chst get(); // blocks
   Chst tryGet(); // noblocks
   Chst waitGet( uint32_t ms = 100 ); // wait + try
   unsigned gets( char *d, unsigned max_len );
   unsigned tryGets( char *d, unsigned max_len );
   void reset() { MuLock lock( mu ); reset_nolock(); };
   void reset_nolock() { s = e = sz = 0; }
  protected:
   bool put_nolock( char c );
   Chst get_nolock();
   unsigned puts_nolock( const char *s );
   unsigned puts_nolock( const char *s, unsigned l );
   unsigned gets_nolock( char *d, unsigned max_len );
  protected:
   char *b;
   const unsigned cap;        //* capacity
   volatile unsigned sz = 0;  //* size
   volatile unsigned s  = 0;  //* start index (over the head)
   volatile unsigned e  = 0;  //* end index
   Mu_t mu = Mu_init;
};

#endif

