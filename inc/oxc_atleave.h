#ifndef _OXC_ATLEAVE_H
#define _OXC_ATLEAVE_H

#include <concepts>


//* helper template class to auto decrement values at function exit
template<std::integral T>
class AutoIncDec {
  public:
   explicit AutoIncDec( T &av ) : v(av) { ++v; }
   ~AutoIncDec() { --v; }
  private:
   T &v;
};

//* helper class to restore value at block exit
template<typename T>
class RestoreAtLeave {
  public:
   explicit RestoreAtLeave( T &av )  : ov(av), v(av) {}
   RestoreAtLeave( T &av, T newval ) : ov(av), v(av) { v = newval; }
   ~RestoreAtLeave() { v = ov; }
  private:
   T ov;
   T &v;
};

template<std::invocable F>
class DoAtLeave {
  public:
   explicit DoAtLeave( F af )  : f(af) {}
   ~DoAtLeave() { f(); }
  private:
   F f;
};

#endif

