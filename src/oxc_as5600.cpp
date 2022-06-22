#include <oxc_as5600.h>


uint16_t AS5600::getAngle()
{
  uint16_t v =  getReg( reg_angle_high );
  int16_t dv = (int16_t)v - (int16_t)old_val;
  if( dv > jumpVal ) {
    --n_turn;
  } else if( dv < -jumpVal ) {
    ++n_turn;
  }
  old_val = v;
  return v;
};






