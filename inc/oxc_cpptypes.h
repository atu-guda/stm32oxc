#ifndef _OXC_CPPTYPES_H
#define _OXC_CPPTYPES_H
// usefull aliases

#include <span>
#include <expected>

#include <oxc_types.h>

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

using exprc_int_t    = std::expected<int,       ReturnCode>;
using exprc_uint_t   = std::expected<unsigned,  ReturnCode>;
using exprc_int8_t   = std::expected<int8_t,    ReturnCode>;
using exprc_uint8_t  = std::expected<uint8_t,   ReturnCode>;
using exprc_int16_t  = std::expected<int16_t,   ReturnCode>;
using exprc_uint16_t = std::expected<uint16_t,  ReturnCode>;
using exprc_int32_t  = std::expected<int32_t,   ReturnCode>;
using exprc_uint32_t = std::expected<uint32_t,  ReturnCode>;

#endif

