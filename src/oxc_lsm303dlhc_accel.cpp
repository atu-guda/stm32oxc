#include <oxc_lsm303dlhc_accel.h>

bool LSM303DHLC_Accel::check_id()
{
  uint8_t v;
  int rc = dev.recv_reg1( reg_id, &v, 1, addr );
  if( rc != 1 || v != id_resp ) {
    return false;
  }
  return true;
}

bool LSM303DHLC_Accel::init( Ctl4_val c4, Ctl2_val c2, Ctl1_val c1 )
{
  dev.resetDev();
  if( !check_id() ) {
    return false;
  }
  uint8_t v = c1;
  int rc = dev.send_reg1( reg_ctl1, v, addr );
  if( rc != 1 ) {
    return false;
  }
  v = c2;
  rc = dev.send_reg1( reg_ctl2, v, addr );
  if( rc != 1 ) {
    return false;
  }
  v = c4;
  rc = dev.send_reg1( reg_ctl4, v, addr );
  if( rc != 1 ) {
    return false;
  }
  return true;
}


void LSM303DHLC_Accel::rebootMem()
{
  uint8_t v;
  int rc = dev.recv_reg1( reg_ctl5, &v, 1, addr );
  if( rc < 1 ) { return; };
  v |= 0x80; // reboot cmd;
  dev.send_reg1( reg_ctl5, v, addr );
}

int16_t LSM303DHLC_Accel::getReg( uint8_t reg )
{
  int32_t v;
  int rc = dev.recv_reg1( reg, (uint8_t*)(&v), 2, addr );
  if( rc < 1 ) {
    return 0;
  }
  // v = rev16( v ); // swap bytes in 16-bits
  return (int16_t)(v);
}

void LSM303DHLC_Accel::getRegs( uint8_t reg1, uint8_t n, int16_t *data )
{
  int rc = dev.recv_reg1( reg1 | 0x80 , (uint8_t*)(data), 2*n, addr );
  if( rc < 1 ) {
    return;
  }
  // rev16( (uint16_t*)data, n );
}



