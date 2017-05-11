#include <oxc_lsm303dlhc_accel.h>

bool LSM303DHLC_Accel::check_id()
{
  uint8_t v;
  int rc = recv_reg1( reg_id, &v, 1 );
  if( rc != 1 || v != id_resp ) {
    return false;
  }
  return true;
}

bool LSM303DHLC_Accel::init( Ctl4_val c4, Ctl2_val c2, Ctl1_val c1 )
{
  resetDev();
  if( !check_id() ) {
    return false;
  }
  uint8_t v = c1;
  int rc = send_reg1_8bit( reg_ctl1, v );
  if( rc != 1 ) {
    return false;
  }
  v = c2;
  rc = send_reg1_8bit( reg_ctl2, v );
  if( rc != 1 ) {
    return false;
  }
  v = c4;
  rc = send_reg1_8bit( reg_ctl4, v );
  if( rc != 1 ) {
    return false;
  }
  return true;
}


void LSM303DHLC_Accel::rebootMem()
{
  uint8_t v;
  int rc = recv_reg1( reg_ctl5, &v, 1 );
  if( rc < 1 ) { return; };
  v |= 0x80; // reboot cmd;
  send_reg1_8bit( reg_ctl5, v );
}

int16_t LSM303DHLC_Accel::getReg( uint8_t reg )
{
  return recv_reg1_16bit( reg, 0 );
}

void LSM303DHLC_Accel::getRegs( uint8_t reg1, uint8_t n, int16_t *data )
{
  recv_reg1_16bit_n( reg1 | 0x80 , (uint16_t*)(data), n );
}



