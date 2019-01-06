#include <oxc_ads1115.h>


//                                  [0]   [1]   [2]   [3]  [4]   [6]   [6]  [7]
const int ADS1115::scale_mv[8] = { 6114, 4096, 2048, 1024, 512,  256,  256, 256 };

uint16_t ADS1115::readReg( uint8_t reg )
{
  if( reg > 3 ) {
    return 0;
  }
  uint16_t v = recv_reg1_16bit_rev( reg, 0 );
  return v;
}

bool ADS1115::writeReg( uint8_t reg, uint16_t val )
{
  if( reg > 3 ) {
    return false;
  }
  return send_reg1_16bit_rev( reg, val ) == 2;
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

int ADS1115::getOneShotNch( uint8_t s_ch, uint8_t e_ch, int16_t *d )
{
  if( !d || s_ch > 3 || e_ch > 3 || e_ch < s_ch ) {
    return 0;
  }
  static const uint16_t ch_bits[n_ch_max] = { cfg_in_0, cfg_in_1, cfg_in_2, cfg_in_3 };
  int n = 0;
  uint16_t cfg_old  = cfg_val;
  uint16_t cfg_base = cfg_val & ~cfg_in_mask;
  for( uint8_t ch = s_ch; ch <= e_ch; ++ch ) {
    uint16_t cvx = cfg_base | ch_bits[ch];
    writeReg( reg_cfg, cvx );
    delay_mcs( 500 );
    writeReg( reg_cfg, cvx | cfg_os);
    for( int i=0; i<200; ++i ) { // for 8 sps 200 ms is good
      if(  getDeviceCfg() & cfg_os  ) {
        d[n] = getContValue();
        ++n;
        break;
      }
      delay_ms( 1 );
    }
  }
  setCfg( cfg_old );
  return n;
}

int ADS1115::getScale_mV() const
{
  int idx = ( cfg_val >> 12 ) && 0x07; // get PGA index
  return scale_mv[idx];
}

