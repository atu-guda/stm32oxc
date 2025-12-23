#ifndef _OXC_REFPTR_H
#define _OXC_REFPTR_H

// simple template class: represent array of pointers as array of refs

#include <cstddef>
#include <iterator>

using std::size_t;

template <typename T, size_t N>
struct RefPtr {
  T* const (&p)[N];
  constexpr RefPtr( T *const (&p_)[N] ) : p( p_ ) {}
  [[nodiscard]] inline T& operator[]( size_t i ) const { return *p[i]; }
  struct Iterator {
    T* const* p;
    T& operator*() const { return **p; }
    Iterator& operator++() { ++p; return *this; }
    const Iterator operator++(int) { auto t = *this; ++p; return t; }
    friend bool operator!=( Iterator a, Iterator b ) { return ( a.p != b.p ); }; // friend for simmetry, cheap arg
    friend bool operator==( Iterator a, Iterator b ) { return ! operator!=( a, b ); };
  };
  [[nodiscard]] constexpr Iterator begin() const { return {p}; };
  [[nodiscard]] constexpr Iterator end()   const { return {p+N}; };
  [[nodiscard]] constexpr size_t   size()  const { return N; };
};

#endif

// vim: path=.,/usr/share/stm32cube/inc
