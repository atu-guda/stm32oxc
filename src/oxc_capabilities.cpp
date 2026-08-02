#include <ranges>

#include <oxc_capabilities.h>

using namespace oxc;


ReturnCode oxc::RoboObject::init() noexcept
{
  sta = doInit();
  return sta;
}


ReturnCode oxc::RoboObject::measure() noexcept
{
  sta = doMeasure();
  return sta;
}


ReturnCode oxc::RoboObject::think() noexcept
{
  sta = doThink();
  return sta;
}


ReturnCode oxc::RoboObject::commit() noexcept
{
  sta = doCommit();
  if( sta.isOk() ) {
    dirty = false;
  }
  return sta;
}




ReturnCode oxc::IoCapability::setVals( cint32_t_span vs ) noexcept
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

ReturnCode oxc::IoCapability::getVals( int32_t_span vs ) noexcept
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

ReturnCode oxc::IoRoboCapability::setVal( size_t ch, int32_t v ) noexcept
{
  if( ch >= sz ) {
    return rcErr;
  }
  if( v != buf[ch] ) {
    buf[ch] = v;
    dirty = true;
  }
  return rcOk;
}

int32_t_er oxc::IoRoboCapability::getVal( size_t ch ) noexcept
{
  if( ch >= sz ) {
    return std::unexpected( rcErr );
  }
  return buf[ch];
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

