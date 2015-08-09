#include <oxc_mpu6050.h>


int16_t MPU6050::getReg( uint8_t reg )
{
  int32_t v;
  int rc = recv_reg1( reg, (uint8_t*)(&v), 2 );
  if( rc < 1 ) {
    return 0;
  }
  v = rev16( v ); // swap bytes in 16-bits
  return (int16_t)(v);
}

void MPU6050::getRegs( uint8_t reg1, uint8_t n, int16_t *data )
{
  int rc = recv_reg1( reg1, (uint8_t*)(data), 2*n );
  if( rc < 1 ) {
    return;
  }
  rev16( (uint16_t*)data, n );
}

void MPU6050::init()
{
  setAccScale( accs_2g );
  setGyroScale( gyros_250 );
  wake( pll_gyro_x );
}


