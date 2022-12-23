#include <oxc_base.h>
#include <oxc_ticker.h>

void OxcTicker::start()
{
  next = HAL_GetTick() + *pw * q;
};

bool OxcTicker::isTick()
{
  int c = HAL_GetTick();
  if( c < next ) {
    return false;
  }
  next += *pw * q;
  return true;
}

