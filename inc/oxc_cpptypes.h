#ifndef _OXC_CPPTYPES_H
#define _OXC_CPPTYPES_H
// usefull aliases

#include <span>
#include <expected>

#include <oxc_types.h>

template<typename T> class _ShowType; // to output deducted type
//                        // _ShowType< decltype(XXXX) > xType;


using byte_span = std::span<uint8_t>;
using cbyte_span = std::span<const uint8_t>;

using exprc_int_t  = std::expected<int,ReturnCode>;
using exprc_uint_t = std::expected<unsigned,ReturnCode>;
using exprc_int8_t  = std::expected<int8_t,ReturnCode>;
using exprc_uint8_t = std::expected<uint8_t,ReturnCode>;
using exprc_int16_t  = std::expected<int16_t,ReturnCode>;
using exprc_uint16_t = std::expected<uint16_t,ReturnCode>;

#endif

