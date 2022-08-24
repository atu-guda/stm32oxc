#include <oxc_hmc5983.h>

const int32_t HMC5983::mcGa_LSbs[n_scales] {
 730, 920, 1220, 1520, 2270, 2560, 3030, 4350 // mcGa/LSb
};

const char  HMC5983::scale_bits[n_scales]  {
  crb_scale_0_88, crb_scale_1_3, crb_scale_1_9, crb_scale_2_5,
  crb_scale_4_0,  crb_scale_4_7, crb_scale_5_6, crb_scale_8_1
};

bool HMC5983::init( CRA odr, Scales scale )
{
  uint8_t buf[4];
  mcGa_LSb = 0;
  buf[0] = odr | cra_temp_en;
  if( scale >= n_scales ) {
    scale = scale_8_1;
  }
  buf[1] = scale_bits[scale];
  buf[2] = mode_idle;

  resetDev();

  if( send_reg1( reg_cra, buf, 3 ) != 3 ) {
    return false;
  }
  mcGa_LSb = mcGa_LSbs[scale];
  return true;
}



int16_t HMC5983::getReg( uint8_t reg )
{
  return recv_reg1_16bit_rev( reg );
}

bool HMC5983::getRegs( uint8_t reg1, uint8_t n, int16_t *data )
{
  return recv_reg1_16bit_n_rev( reg1, (uint16_t*)(data), n ) == n;
}

// workhorse for read1 and readNextAuto
bool HMC5983::wait_read( int32_t wait_ms )
{
  // init is by read1 or startAuto
  uint8_t sta = status_rdy;
  if( wait_ms >= 0 ) {
    for( int i=0; i< max_loops; ++i ) {
      delay_ms( wait_ms );
      sta = 0;
      recv_reg1( reg_sr, &sta, 1 );
      if( sta & status_rdy ) {
        break;
      }
    }
  }
  if( ! (sta & status_rdy) ) {
    return false;
  }

  if( ! getRegs( reg_m_xh, 3, regsXYZ ) ) {
    return false;
  }
  valsXYZ[0] = regsXYZ[0] * mcGa_LSb;
  valsXYZ[1] = regsXYZ[1] * mcGa_LSb;
  valsXYZ[2] = regsXYZ[2] * mcGa_LSb;
  return true;
}

bool HMC5983::read1( int32_t wait_ms )
{
  regsXYZ[0] = regsXYZ[1] = regsXYZ[2] = 0;
  if( send_reg1_8bit( reg_mode, mode_single ) != 1 ) {
    return false;
  }
  return wait_read( wait_ms );
}

bool HMC5983::startAuto()
{
  if( send_reg1_8bit( reg_mode, mode_cont ) != 1 ) {
    return false;
  }
  return true;
}

// TODO: BUG: recurse!?!
// bool HMC5983::readNextAuto( int32_t wait_ms )
// {
//   return readNextAuto( wait_ms );
// }

bool HMC5983::stopAuto()
{
  if( send_reg1_8bit( reg_mode, mode_idle ) != 1 ) {
    return false;
  }
  return true;
}

