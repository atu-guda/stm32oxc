#include <oxc_as5600.h>


using namespace oxc;

uint16_t AS5600::getAngle()
{
  uint16_t v = recv_reg1_16bit_rev( reg_angle_high, 0xFFFF ); // 0xFFFF - value if error, sensor is 12-bit
  if( v == 0xFFFF ) {
    return v;
  }

  int16_t dv = (int16_t)v - (int16_t)old_val;
  if( dv > jumpVal ) {
    --n_turn;
  } else if( dv < -jumpVal ) {
    ++n_turn;
  }
  old_val = v;
  return v;
};



