#ifndef _OXC_TYPES_H
#define _OXC_TYPES_H

#include <cstdint>
#include <span>
#include <expected>
#include <optional>

#define UNUSED_ARG __attribute__((unused))

#if defined ( __GNUC__ )
  #ifndef __weak
    #define __weak   __attribute__((weak))
  #endif
  #ifndef __packed
    #define __packed __attribute__((__packed__))
  #endif
#endif

#define PORT_BITS 16
#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000

#define BIT0M  0x0001_mask
#define BIT1M  0x0002_mask
#define BIT2M  0x0004_mask
#define BIT3M  0x0008_mask
#define BIT4M  0x0010_mask
#define BIT5M  0x0020_mask
#define BIT6M  0x0040_mask
#define BIT7M  0x0080_mask
#define BIT8M  0x0100_mask
#define BIT9M  0x0200_mask
#define BIT10M 0x0400_mask
#define BIT11M 0x0800_mask
#define BIT12M 0x1000_mask
#define BIT13M 0x2000_mask
#define BIT14M 0x4000_mask
#define BIT15M 0x8000_mask

#define BAD_ADDR ((void*)(0xFFFFFFFF))
#ifndef NL
  #define NL "\n"
#endif

#ifndef   __IO
  #define __IO volatile
#endif
typedef __IO uint32_t reg32;
typedef const char *const ccstr;
typedef uint32_t mu_t; // mutex_t alike

typedef void (*AuxTickFun)(void);

namespace oxc {

struct ReturnCode {
  enum ReturnCodeNum {
    rcnOk    =  0,
    rcnInfo  =  1,
    rcnWarn  =  2,
    rcnExtra =  3,
    rcnEnd   =  4,
    rcnErr   = 10,
    rcnFatal = 20
  };

  uint16_t code;
  uint16_t data;

  constexpr ReturnCode( uint16_t co, uint16_t da = 0 ) noexcept : code( co ), data( da ) {};
  constexpr bool isOk()      const noexcept { return code <  rcnErr;   }
  constexpr bool isEnd()     const noexcept { return code == rcnEnd;   }
  constexpr bool isWarning() const noexcept { return code == rcnWarn;  }
  constexpr bool isError()   const noexcept { return code >= rcnErr;   }
  constexpr bool isFatal()   const noexcept { return code >= rcnFatal; }
  constexpr static ReturnCode makeErr( uint16_t da = 0 ) noexcept { return ReturnCode{ rcnErr , da }; }

  constexpr bool operator==( enum ReturnCodeNum rcn ) const noexcept {
    return code == rcn && data == 0;
  }
  constexpr bool operator!=( enum ReturnCodeNum rcn ) const noexcept {
    return !( *this == rcn );
  }
  constexpr bool operator==( const ReturnCode &rhs ) const noexcept = default;
  constexpr uint32_t toUint32() const { return code << 16 | data; }
  constexpr explicit operator bool() const noexcept { return isOk(); }

  void ifErr( void(*f)(void)  ) { if( isError() ) { f(); } }

};
static_assert( sizeof(ReturnCode) == sizeof(uint32_t) );

inline constexpr ReturnCode rcOk    { ReturnCode::rcnOk,    0 };
inline constexpr ReturnCode rcWarn  { ReturnCode::rcnWarn,  0 };
inline constexpr ReturnCode rcEnd   { ReturnCode::rcnEnd,   0 };
inline constexpr ReturnCode rcErr   { ReturnCode::rcnErr ,  0 };
inline constexpr ReturnCode rcFatal { ReturnCode::rcnFatal, 0 };


// usefull suffixes for ints
constexpr std::uint8_t operator""_u8(unsigned long long v) {
  return static_cast<std::uint8_t>(v);
}
constexpr std::int8_t operator""_i8(unsigned long long v) {
  return static_cast<std::int8_t>(v);
}
constexpr std::uint16_t operator""_u16(unsigned long long v) {
  return static_cast<std::uint16_t>(v);
}
constexpr std::int16_t operator""_i16(unsigned long long v) {
  return static_cast<std::int16_t>(v);
}
constexpr std::uint32_t operator""_u32(unsigned long long v) {
  return static_cast<std::uint32_t>(v);
}
constexpr std::int32_t operator""_i32(unsigned long long v) {
  return static_cast<std::int32_t>(v);
}
constexpr std::uint32_t operator""_u64(unsigned long long v) {
  return static_cast<std::uint64_t>(v);
}
constexpr std::int32_t operator""_i64(unsigned long long v) {
  return static_cast<std::int64_t>(v);
}
// uint32_t v = 42_u32;

template<typename T> class _ShowType; // to output deducted type
//                        // _ShowType< decltype(XXXX) > xType;


using byte_span      = std::span<uint8_t>;
using cbyte_span     = std::span<const uint8_t>;
using char_span      = std::span<char>;
using cchar_span     = std::span<const char>;
using int_span       = std::span<int>;
using cint_span      = std::span<const int>;
using int16_t_span   = std::span<int16_t>;
using cint16_t_span  = std::span<const int16_t>;
using uint16_t_span  = std::span<uint16_t>;
using cuint16_t_span = std::span<const uint16_t>;
using int32_t_span   = std::span<int32_t>;
using cint32_t_span  = std::span<const int32_t>;
using uint32_t_span  = std::span<uint32_t>;
using cuint32_t_span = std::span<const uint32_t>;

using exprc_int_t    = std::expected<int,      oxc::ReturnCode>;
using exprc_uint_t   = std::expected<unsigned, oxc::ReturnCode>;
using exprc_int8_t   = std::expected<int8_t,   oxc::ReturnCode>;
using exprc_uint8_t  = std::expected<uint8_t,  oxc::ReturnCode>;
using exprc_int16_t  = std::expected<int16_t,  oxc::ReturnCode>;
using exprc_uint16_t = std::expected<uint16_t, oxc::ReturnCode>;
using exprc_int32_t  = std::expected<int32_t,  oxc::ReturnCode>;
using exprc_uint32_t = std::expected<uint32_t, oxc::ReturnCode>;
using exprc_int64_t  = std::expected<int64_t,  oxc::ReturnCode>;
using exprc_uint64_t = std::expected<uint64_t, oxc::ReturnCode>;

using int_t_o    = std::optional<int     >;
using uint_t_o   = std::optional<unsigned>;
using int8_t_o   = std::optional<int8_t  >;
using uint8_t_o  = std::optional<uint8_t >;
using int16_t_o  = std::optional<int16_t >;
using uint16_t_o = std::optional<uint16_t>;
using int32_t_o  = std::optional<int32_t >;
using uint32_t_o = std::optional<uint32_t>;
using int64_t_o  = std::optional<int64_t >;
using uint64_t_o = std::optional<uint64_t>;

// template< typename DevType, std::uintptr_t addr_ >
// struct DevPtr
// {
//   static constexpr auto addr { addr_ };
//   static volatile DevType& pdev() noexcept {
//     return *reinterpret_cast<volatile DevType*>( addr );
//   }
// };

// usage like ????
// using Tim1 = DevPtr<TIM_TypeDef, TIM1_BASE>;

} // namespace oxc

#endif

