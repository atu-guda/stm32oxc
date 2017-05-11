#include <oxc_lsm303dlhc_mag.h>


bool LSM303DHLC_Mag::init( uint8_t odr, uint8_t sens )
{
  resetDev();
  if( send_reg1_8bit( reg_cra, odr ) != 1 ) {
    return false;
  }
  if( send_reg1_8bit( reg_crb, sens ) != 1 ) {
    return false;
  }
  if( send_reg1_8bit( reg_mode, 0 ) != 1 ) { // 0 = continous 1=single_shot
    return false;
  }
  return true;
}



int16_t LSM303DHLC_Mag::getReg( uint8_t reg )
{
  return recv_reg1_16bit_rev( reg, 0 );
}

void LSM303DHLC_Mag::getRegs( uint8_t reg1, uint8_t n, int16_t *data )
{
  recv_reg1_16bit_n_rev( reg1 | 0x80 , (uint16_t*)(data), 2 );
}



