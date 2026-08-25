#include <ranges>

#include <oxc_capabilities.h>

using namespace oxc;

int32_t_er oxc::PinsCapability::getVal( size_t ch ) noexcept
{
  switch( ch ) {
    case ch_read  : return pins.read();
    case ch_write : return pins.readwr();
  }
  return std::unexpected( rcErr );
}

ReturnCode oxc::PinsCapability::setVal( size_t ch, int32_t v ) noexcept
{
  switch( ch ) {
    case ch_write     : pins.write(     v ); return rcOk;
    case ch_set       : pins.set(       v ); return rcOk;
    case ch_reset     : pins.reset(     v ); return rcOk;
    case ch_toggle    : pins.toggle(    v ); return rcOk;
    case ch_setbit    : pins.setbit(    v ); return rcOk;
    case ch_resetbit  : pins.resetbit(  v ); return rcOk;
    case ch_togglebit : pins.togglebit( v ); return rcOk;
  }
  return rcErr;
}


int32_t_er oxc::PinCapability::getVal( size_t ch ) noexcept
{
  switch( ch ) {
    case ch_read  : return pin.read();
    case ch_write : return pin.readwr();
  }
  return std::unexpected( rcErr );
}


ReturnCode oxc::PinCapability::setVal( size_t ch, int32_t v ) noexcept
{
  switch( ch ) {
    case ch_write     : pin.write( v ); return rcOk;
    case ch_set       : pin.set();      return rcOk;
    case ch_reset     : pin.reset();    return rcOk;
    case ch_toggle    : pin.toggle();   return rcOk;
  }
  return rcErr;
}


