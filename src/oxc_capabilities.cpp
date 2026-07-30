#include <ranges>

#include <oxc_capabilities.h>

using namespace oxc;

ReturnCode oxc::IOCapability::setVals( cint32_t_span vs ) noexcept
{
  for( auto [ch,v] : std::views::enumerate( vs )  ) {
    if( (size_t)ch >= size() ) {
      break;
    }
    if( auto rc = setVal( ch, v ); rc.isError() ) {
      return rc;
    }
  }
  return rcOk;
}

ReturnCode oxc::IOCapability::getVals( int32_t_span vs ) noexcept
{
  for( auto [ch,v] : std::views::enumerate( vs )  ) {
    if( (size_t)ch >= size() ) {
      break;
    }
    auto rcv = getVal( ch );
    if( !rcv ) {
      return rcv.error();
    }
    vs[ch] = rcv.value();
  }
  return rcOk;
}


ReturnCode oxc::AnalogCapability::setValFs( cfloat_span vs ) noexcept
{
  for( auto [ch,v] : std::views::enumerate( vs )  ) {
    if( (size_t)ch >= size() ) {
      break;
    }
    if( auto rc = setValF( ch, v ); rc.isError() ) {
      return rc;
    }
  }
  return rcOk;
}

ReturnCode oxc::AnalogCapability::getValFs( float_span vs ) noexcept
{
  for( auto [ch,v] : std::views::enumerate( vs )  ) {
    if( (size_t)ch >= size() ) {
      break;
    }
    auto rcv = getValF( ch );
    if( !rcv ) {
      return rcv.error();
    }
    vs[ch] = rcv.value();
  }
  return rcOk;
}

