#include <oxc_lsm303dlhc_mag.h>


bool LSM303DHLC_Mag::init( uint8_t odr, uint8_t sens )
{
  resetDev();
  if( dev.send_reg1( reg_cra, odr, addr ) != 1 ) {
    return false;
  }
  if( dev.send_reg1( reg_crb, sens, addr ) != 1 ) {
    return false;
  }
  if( dev.send_reg1( reg_mode, 0, addr ) != 1 ) { // 0 = continous 1=single_shot
    return false;
  }
  return true;
}



int16_t LSM303DHLC_Mag::getReg( uint8_t reg )
{
  int32_t v;
  int rc = dev.recv_reg1( reg, (uint8_t*)(&v), 2, addr );
  if( rc < 1 ) {
    return 0;
  }
  v = rev16( v ); // swap bytes in 16-bits
  return (int16_t)(v);
}

void LSM303DHLC_Mag::getRegs( uint8_t reg1, uint8_t n, int16_t *data )
{
  int rc = dev.recv_reg1( reg1 | 0x80 , (uint8_t*)(data), 2*n, addr );
  if( rc < 1 ) {
    return;
  }
  rev16( (uint16_t*)data, n );
}



