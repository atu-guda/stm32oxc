#include <ranges>

// #include <oxc_debug1.h>

#include <oxc_bitops.h>
#include <oxc_robocaps.h>

using namespace oxc;


ReturnCode oxc::RoboObject::init() noexcept
{
  if( ! ( flags & noInit ) ) {
    sta = doInit();
  }
  dirty = 0;
  return sta;
}


ReturnCode oxc::RoboObject::measure() noexcept
{
  if( ! ( flags & noMeasure ) ) {
    sta = doMeasure();
  }
  return sta;
}


ReturnCode oxc::RoboObject::think() noexcept
{
  if( ! ( flags & noThink ) ) {
    sta = doThink();
  }
  return sta;
}


ReturnCode oxc::RoboObject::commit() noexcept
{
  if( ! ( flags & noCommit ) ) {
    sta = doCommit();
    if( sta.isOk() ) {
      dirty = 0;
    }
  }
  return sta;
}



int32_t_er oxc::PinsRoboCapability::getVal( size_t ch ) noexcept
{
  switch( ch ) {
    case ch_read  : return vv[0];
    case ch_write : return vv[1];
  }
  return std::unexpected( rcErr );
}

ReturnCode oxc::PinsRoboCapability::setVal( size_t ch, int32_t v ) noexcept
{
  auto vvo = vv[1];
  switch( ch ) {
    case ch_write     : vv[1]  =  v;  break;
    case ch_set       : vv[1] |=  v;  break;
    case ch_reset     : vv[1] &= ~v;  break;
    case ch_toggle    : vv[1] ^=  v;  break;
    case ch_setbit    : set_bit(    vv[1], v ); break;
    case ch_resetbit  : reset_bit(  vv[1], v ); break;
    case ch_togglebit : toggle_bit( vv[1], v ); break;
    default           : return rcErr;
  }
  if( vvo != vv[1] ) {
    dirty |= ch_w_bit;
  }
  return rcOk;
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
  auto vvo = vv[1];
  switch( ch ) {
    case ch_write     : vv[1] =  v;     break;
    case ch_set       : vv[1] =  1;     break;
    case ch_reset     : vv[1] =  0;     break;
    case ch_toggle    : vv[1] = !vv[1]; break;
    default           : return rcErr;
  }
  if( vvo != vv[1] ) {
    dirty |= ch_w_bit;
  }
  return rcOk;
}




