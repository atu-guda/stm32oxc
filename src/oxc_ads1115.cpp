#include <oxc_ads1115.h>


//                                  [0]   [1]   [2]   [3]  [4]   [6]   [6]  [7]
const int ADS1115::scale_mv[8] = { 6114, 4096, 2048, 1024, 512,  256,  256, 256 };

uint16_t ADS1115::readReg( uint8_t reg )
{
  reg &= 0x03; // limit to 0-3
  uint16_t v = recv_reg1_16bit_rev( reg, 0 );
  return v;
}

bool ADS1115::writeReg( uint8_t reg, uint16_t val )
{
  reg &= 0x03; // limit to 0-3
  return send_reg1_rev( reg, val ) == 2;
}

int16_t ADS1115::getOneShot()
{
  uint16_t cvx = cfg_val | cfg_os;
  writeReg( reg_cfg, cvx );
  for( int i=0; i<200; ++i ) { // for 8 sps 200 ms is good
    if(  getDeviceCfg() & cfg_os  ) {
      return getContValue();
    }
    delay_ms( 1 );
  }
  return -1;
}

int ADS1115::getScale_mV() const
{
  int idx = ( cfg_val >> 12 ) && 0x07; // get PGA index
  return scale_mv[idx];
}

