#ifndef _OXC_BITOPS_H
#define _OXC_BITOPS_H

#include <concepts>

inline constexpr uint32_t make_bit_mask( uint8_t start, uint8_t n )
{
  return ( (uint32_t)(0xFFFFFFFF) << ( 32 - n ) ) >> ( 32 - n - start );
}

inline constexpr uint32_t make_bit_mask_left( uint8_t n )
{
  return ( (uint32_t)(0xFFFFFFFF) >> ( 32 - n ) );
}

inline constexpr uint16_t make_gpio_mask( uint8_t start, uint8_t n )
{
  return (uint16_t) make_bit_mask( start, n );
}

inline bool check_bit( uint32_t v, uint8_t pos )
{
  return v & ( 1u << pos );
}


template<std::integral T>
inline void set_bit( T &v, uint8_t pos )
{
  v |= 1 << pos;
}

template<std::integral T>
inline void set_bits( T &v, uint8_t pos, uint8_t n )
{
  v |= make_bit_mask( pos, n );
}

template<std::integral T>
inline void reset_bit( T &v, uint8_t pos )
{
  v &= ~( 1u << pos );
}

template<std::integral T>
inline void reset_bits( T &v, uint8_t pos, uint8_t n )
{
  v &= ~ make_bit_mask( pos, n );
}

template<std::integral T>
inline void replace_bits( T &v, uint8_t pos, uint8_t n, uint32_t bits )
{
  typename std::remove_volatile<T>::type t = v;
  t &= ~ make_bit_mask( pos, n );
  t |= ( bits << pos );
  v = t;
}

#endif
// vim: path=.,/usr/share/stm32cube/inc
