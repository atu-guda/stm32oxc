#include "endstopgpio.h"

bool EndStopGpioPos::is_clear_for_dir( int dir ) const
{
  if( dir == 0 || ( v & mainBits ) == mainBits ) { // no move or all clear
    return true;
  }
  if( dir >  0 && ( v & plusBit  ) ) { // forward and ep+ clear
    return true;
  }
  if( dir <  0 && ( v & minusBit ) ) { // backward and ep- clear
    return true;
  }
  return false;
}

