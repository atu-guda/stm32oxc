#include <oxc_as5600.h>


using namespace oxc;

uint16_t_o AS5600::getAngle_o()
{
  auto vo = recv_reg1_16bit_rev_o( reg_angle_high );
  if( !vo ) {
    return {};
  }
  auto v = vo.value();

  int16_t dv = (int16_t)v - (int16_t)old_val;
  if( dv > jumpVal ) {
    --n_turn;
  } else if( dv < -jumpVal ) {
    ++n_turn;
  }
  old_val = v;
  return v;
};





