#include <ranges>

#include <oxc_robocaps.h>

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
  // BUG: where to store dirty?
  // if( sta.isOk() ) {
  //   dirty = 0;
  // }
  return sta;
}





ReturnCode oxc::IoRoboCapability::setVal( size_t ch, int32_t v ) noexcept
{
  if( ch >= sz ) {
    return rcErr;
  }
  if( v != iobuf[ch] ) {
    iobuf[ch] = v;
    dirty |= (1<<ch);
  }
  return rcOk;
}

int32_t_er oxc::IoRoboCapability::getVal( size_t ch ) noexcept
{
  if( ch >= sz ) {
    return std::unexpected( rcErr );
  }
  return iobuf[ch];
}



