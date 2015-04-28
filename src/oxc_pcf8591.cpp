#include <oxc_pcf8591.h>

#define I2C_TO 100

int PCF8591::setMode( uint8_t mode )
{
  cfg_reg = mode;
  return HAL_I2C_Master_Transmit( &i2ch, addr2, &cfg_reg, 1, I2C_TO );
}

int PCF8591::getIn( uint8_t *d, int sz )
{
  return HAL_I2C_Master_Receive( &i2ch, addr2, d, sz, I2C_TO );
}

int PCF8591::setOut( uint8_t v )
{
  uint8_t ou[2] = { cfg_reg, v };
  return HAL_I2C_Master_Transmit( &i2ch, addr2, ou, 2, I2C_TO );
}


#undef I2C_TO

