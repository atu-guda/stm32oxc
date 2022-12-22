#ifndef _OXC_EV_H
#define _OXC_EV_H

class Chst { // char + status
  public:
   enum {
     st_good = 0, st_full = 1, st_empty = 2, st_lock = 4
   };
   explicit Chst( char ch ) : c( ch ), st( st_good ) {};
   Chst( char ch, uint8_t a_st ) : c( ch ), st( a_st ) {};
   bool good()   const noexcept { return st == st_good;  }
   bool full()   const noexcept { return st == st_full;  }
   bool empty()  const noexcept { return st == st_empty; }
   bool locked() const noexcept { return st == st_lock;  }

   char c;
   uint8_t st;
};
static_assert( sizeof(Chst) == 2, "Bad size of Chst struct, !=2" );

#endif

