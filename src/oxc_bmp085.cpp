#include <oxc_bmp085.h>

// #define BMP_DEBUG 1
#undef BMP_DEBUG


bool BMP085::readCalibrData()
{
#ifndef BMP_DEBUG
  int rc = recv_reg1( reg_calibr_start, (uint8_t*)(&calibr), sizeof(calibr) );
  if( rc < (int)sizeof(calibr) ) {
    return false;
  }
  rev16( (uint16_t*)(&calibr), n_calibr_data );
#else
  calibr.ac1 = 408;   calibr.ac2 = -72;   calibr.ac3 = -14383;
  calibr.ac4 = 32741; calibr.ac5 = 32757; calibr.ac6 =  23153;
  calibr.b1 = 6190;
  calibr.b2 = 4;
  calibr.mb = -32768;
  calibr.mc = -8711;
  calibr.md = 2868;
#endif
  return true;
}

int  BMP085::get_T_uncons( bool do_get )
{
#ifndef BMP_DEBUG
  if( do_get ) {
    send_reg1( reg_cmd,  cmd_read_T );
    delay_ms( t_wait_T );
    uint8_t tt[2];
    recv_reg1( reg_out, tt, sizeof(tt) );
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
    send_reg1( reg_cmd, cmd );
    delay_ms( t_wait_P );
    uint8_t tp[3];
    recv_reg1( reg_out, tp, sizeof(tp) );
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


