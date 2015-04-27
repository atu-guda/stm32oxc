#include <oxc_bmp085.h>

// #define BMP_DEBUG 1
#undef BMP_DEBUG

#define I2C_TO 200

void BMP085::readCalibrData()
{
#ifndef BMP_DEBUG
  HAL_I2C_Mem_Read( &i2ch, addr2, reg_calibr_start, I2C_MEMADD_SIZE_8BIT, (uint8_t*)(&calibr), sizeof(calibr), I2C_TO );
  uint16_t *data = (uint16_t*)(&calibr);
  for( uint8_t i=0; i<n_calibr_data; ++i ) { // swap
    uint32_t v = data[i];
    v = __REV16( v );
    data[i] = (uint16_t)(v);
  }
#else
  calibr.ac1 = 408;   calibr.ac2 = -72;   calibr.ac3 = -14383;
  calibr.ac4 = 32741; calibr.ac5 = 32757; calibr.ac6 =  23153;
  calibr.b1 = 6190;
  calibr.b2 = 4;
  calibr.mb = -32768;
  calibr.mc = -8711;
  calibr.md = 2868;
#endif

}

int  BMP085::get_T_uncons( bool do_get )
{
#ifndef BMP_DEBUG
  if( do_get ) {
    uint8_t cmd = cmd_read_T;
    HAL_I2C_Mem_Write( &i2ch, addr2, reg_cmd, I2C_MEMADD_SIZE_8BIT, &cmd, 1, I2C_TO );
    delay_ms( t_wait_T );
    uint8_t tt[2];
    HAL_I2C_Mem_Read( &i2ch, addr2, reg_out, I2C_MEMADD_SIZE_8BIT, tt, sizeof(tt), I2C_TO );
    t_uncons = ( (tt[0]<<8) + tt[1] );
  }
#else
  t_uncons = 27898;
#endif

  return t_uncons;
}

int  BMP085::get_P_uncons( bool do_get )
{
#ifndef BMP_DEBUG
  if( do_get ) {
    uint8_t cmd = cmd_read_P0 | ( oss << 6 );
    HAL_I2C_Mem_Write( &i2ch, addr2, reg_cmd, I2C_MEMADD_SIZE_8BIT, &cmd, 1, I2C_TO );
    delay_ms( t_wait_P );
    uint8_t tp[3];
    HAL_I2C_Mem_Read( &i2ch, addr2, reg_out, I2C_MEMADD_SIZE_8BIT, tp, sizeof(tp), I2C_TO );
    p_uncons = ( (tp[0]<<16) + (tp[1]<<8) + tp[2] ) >> (8-oss);
  }
#else
  p_uncons = 23843 << oss;
#endif

  return p_uncons;
}

void BMP085::getAllCalc( uint8_t a_oss )
{
  oss = a_oss;
  if( oss > 3 ) {
    oss = 3;
  }
  get_T_uncons( true );
  get_P_uncons( true );
  calc();

}

void BMP085::calc()
{
  int x1 = ( ( t_uncons - calibr.ac6 ) * calibr.ac5 ) >> 15;
  // pr_sdx( x1 );
  int x2 = ( (int)(calibr.mc) << 11 ) / ( x1 + calibr.md );
  int b5 = x1 + x2;
  t10 = ( b5 + 8 ) >> 4;

  int b6 = b5 - 4000;
  int b6_2_12 = (b6 * b6) >> 12;
  x1 = ( calibr.b2 * b6_2_12 ) >> 11;
  x2 = ( calibr.ac2 * b6 ) >> 11;
  int x3 = x1 + x2;
  int b3 = ( ( ( (int)(calibr.ac1)*4 + x3 ) << oss ) + 2 ) / 4;

  x1 = ( (int)(calibr.ac3) * b6 ) >> 13;
  x2 = ( calibr.b1 * b6_2_12 ) >> 16;
  x3 = ( ( x1 + x2 ) + 2 ) >> 2;

  int b4 = ( calibr.ac4 * ( (unsigned)(x3 + 32768) ) ) >> 15;
  unsigned b7 = ( (unsigned)(p_uncons) - b3 ) * ( 50000u >> oss );

  if( (unsigned)(b7) < 0x80000000 ) {
    p = ( b7 * 2 ) / b4;
  } else {
    p = ( b7 / b4 ) * 2;
  }
  x1 = ( p >> 8 ) * ( p >> 8 );
  x1 = ( x1 * 3038 ) >> 16;
  x2 = ( -7357 * p ) >> 16;

  p = p + ( ( x1 + x2 + 3791) >> 4);

}

#undef I2C_TO

