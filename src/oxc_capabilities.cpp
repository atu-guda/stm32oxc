#include <ranges>

#include <oxc_capabilities.h>

using namespace oxc;

 // stash:
 //  explicit constexpr IoCapability( size_t sz_, size_t bitsz_, int32_t scale_ ) noexcept :
 // ReturnCode setValF( size_t ch, float v ) noexcept {
 //   return setVal( ch, (int32_t) std::lroundf( v * scale ) );
 // }
 // float_er   getValF( size_t ch ) noexcept {
 //   auto vi_er = getVal( ch );
 //   if( !vi_er ) {
 //     return vi_er;
 //   }
 //   return vi_er.value() / scale;
 // }

// may be useless now

// ReturnCode oxc::IoCapability::setVals( cint32_t_span vs ) noexcept
// {
//   for( auto [ch,v] : std::views::enumerate( vs )  ) {
//     if( (size_t)ch >= size() ) {
//       break;
//     }
//     if( auto rc = setVal( ch, v ); rc.isError() ) {
//       return rc;
//     }
//   }
//   return rcOk;
// }
//
// ReturnCode oxc::IoCapability::getVals( int32_t_span vs ) noexcept
// {
//   for( auto [ch,v] : std::views::enumerate( vs )  ) {
//     if( (size_t)ch >= size() ) {
//       break;
//     }
//     auto rcv = getVal( ch );
//     if( !rcv ) {
//       return rcv.error();
//     }
//     vs[ch] = rcv.value();
//   }
//   return rcOk;
// }
//
//
//
//
// ReturnCode oxc::IoCapability::setValFs( cfloat_span vs ) noexcept
// {
//   for( auto [ch,v] : std::views::enumerate( vs )  ) {
//     if( (size_t)ch >= size() ) {
//       break;
//     }
//     if( auto rc = setValF( ch, v ); rc.isError() ) {
//       return rc;
//     }
//   }
//   return rcOk;
// }
//
//
// ReturnCode oxc::IoCapability::getValFs( float_span vs ) noexcept
// {
//   for( auto [ch,v] : std::views::enumerate( vs )  ) {
//     if( (size_t)ch >= size() ) {
//       break;
//     }
//     auto rcv = getValF( ch );
//     if( !rcv ) {
//       return rcv.error();
//     }
//     vs[ch] = rcv.value();
//   }
//   return rcOk;
// }

