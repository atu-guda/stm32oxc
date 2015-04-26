#include <oxc_mpu6050.h>

#define I2C_TO 100

int MPU6050::sendByteReg( uint16_t reg, uint8_t d )
{
  return HAL_I2C_Mem_Write( &i2ch, addr2, reg, I2C_MEMADD_SIZE_8BIT, &d, 1, I2C_TO );
}

int16_t MPU6050::getReg( uint8_t reg )
{
  int32_t v;
  HAL_I2C_Mem_Read( &i2ch, addr2, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)(&v), 2, I2C_TO );
  v = __REV16( v ); // swap bytes in 16-bits
  return (int16_t)(v);
}

void MPU6050::getRegs( uint8_t reg1, uint8_t n, int16_t *data )
{
  HAL_I2C_Mem_Read( &i2ch, addr2, reg1, I2C_MEMADD_SIZE_8BIT, (uint8_t*)(data), 2*n, I2C_TO );
  for( uint8_t i=0; i<n; ++i ) { // swap
    int32_t v = data[i];
    v = __REV16( v );
    data[i] = (int16_t)(v);
  }
}

void MPU6050::init()
{
  setAccScale( accs_2g );
  setGyroScale( gyros_250 );
  wake( pll_gyro_x );
}

#undef I2C_TO

