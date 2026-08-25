#include <ranges>

#include <oxc_bitops.h>
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
  if( sta.isOk() ) {
    dirty = 0;
  }
  return sta;
}



int32_t_er oxc::RoboPinsCapability::getVal( size_t ch ) noexcept
{
  switch( ch ) {
    case ch_read  : return vv[0];
    case ch_write : return vv[1];
  }
  return std::unexpected( rcErr );
}

ReturnCode oxc::RoboPinsCapability::setVal( size_t ch, int32_t v ) noexcept
{
  switch( ch ) {
    case ch_write     : vv[1]  =  v;  return rcOk;
    case ch_set       : vv[1] |=  v;  return rcOk;
    case ch_reset     : vv[1] |= ~v;  return rcOk;
    case ch_toggle    : vv[1] ^=  v;  return rcOk;
    case ch_setbit    : setbit(    vv[1], v ); return rcOk;
    case ch_resetbit  : resetbit(  vv[1], v ); return rcOk;
    case ch_togglebit : togglebit( vv[1], v ); return rcOk;
  }
  return rcErr;
}




int32_t_er oxc::PinRoboCapability::getVal( size_t ch ) noexcept
{
  switch( ch ) {
    case ch_read  : return vv[0];
    case ch_write : return vv[1];
  }
  return std::unexpected( rcErr );
}


ReturnCode oxc::PinRoboCapability::setVal( size_t ch, int32_t v ) noexcept
{
  switch( ch ) {
    case ch_write     : vv[1] =  v;     return rcOk;
    case ch_set       : vv[1] =  1;     return rcOk;
    case ch_reset     : vv[1] =  0;     return rcOk;
    case ch_toggle    : vv[1] = !vv[1]; return rcOk;
  }
  return rcErr;
}




