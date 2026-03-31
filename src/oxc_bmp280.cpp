#include <oxc_bmp280.h>



bool BMP280::readCalibrData()
{
  return recv_reg1_16bit_n( reg_calibr_start, (uint16_t*)(&calibr.dig_t1), n_calibr_data ) == n_calibr_data;

  // debug from DS
  // auto &c = calibr;
  // c.dig_t1 =  27504;
  // c.dig_t2 =  26435;
  // c.dig_t3 =  -1000;
  // //
  // c.dig_p1 =  36477;
  // c.dig_p2 = -10685;
  // c.dig_p3 =   3024;
  // c.dig_p4 =   2855;
  // c.dig_p5 =    140;
  // c.dig_p6 =     -7;
  // c.dig_p7 =  15500;
  // c.dig_p8 = -14600;
  // c.dig_p9 =   6000;
  //
  // return true;
}

bool BMP280::check_id()
{
  id = 0xFF; // bad read indicator
  if( recv_reg1( reg_id, &id, sizeof(id) ) != sizeof(id) ) {
    return false;
  }
  return id == val_id;
}

bool BMP280::reset()
{
  return( send_reg1_8bit( reg_rst, val_reset ) == 1 );
}

bool BMP280::config( uint8_t cfg, uint8_t ctrl )
{
   return
     ( send_reg1_8bit( reg_config, cfg ) == 1 )
     &&
     ( send_reg1_8bit( reg_ctrl_meas, ctrl ) == 1 );
}

bool BMP280::readData()
{
  return recv_reg1( reg_press_msb, data, sz_data ) == sz_data;
}

void BMP280::calc()
{
  const auto &c = calibr;

  t_raw = get_T_raw();
  // t_raw = 519888; // test

  const int32_t var1 = ( (((t_raw>>3) - ((int32_t)c.dig_t1<<1))) * ((int32_t)c.dig_t2) ) >> 11;
  const int32_t var2 = (((((t_raw>>4) - ((int32_t)c.dig_t1)) * ((t_raw>>4) - ((int32_t)c.dig_t1))) >> 12) *
        ((int32_t)c.dig_t3)) >> 14;
  t_fine = var1 + var2;
  t_100 = (t_fine * 5 + 128) >> 8;

  // P part
  p_fine = 0;
  int64_t v1 = ((int64_t)t_fine) - 128000;
  int64_t v2 = v1 * v1 * (int64_t)c.dig_p6;
  v2 = v2 + ((v1*(int64_t)c.dig_p5)<<17);
  v2 = v2 + (((int64_t)c.dig_p4)<<35);
  v1 = ((v1 * v1 * (int64_t)c.dig_p3)>>8) + ((v1 * (int64_t)c.dig_p2)<<12);
  v1 = (((((int64_t)1)<<47)+v1))*((int64_t)c.dig_p1)>>33;
  if( v1 == 0 ) {
    return;
  }

  p_raw = get_P_raw();
  // p_raw =  415148; // test

  int64_t p = 1048576 - p_raw;
  p = ( ( (p<<31) - v2 ) * 3125 ) / v1;
  v1 = ( ( (int64_t)c.dig_p9 ) * ( p>>13) * ( p>>13) ) >> 25;
  v2 = ( ( (int64_t)c.dig_p8 ) * p) >> 19;
  p = ((p + v1 + v2) >> 8) + (((int64_t)c.dig_p7)<<4);
  p_fine = (int32_t)p;
}

