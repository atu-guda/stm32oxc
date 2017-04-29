#include <oxc_pcf8591.h>


int PCF8591::setMode( uint8_t mode )
{
  cfg_reg = mode;
  return dev.send( cfg_reg, addr );
}

int PCF8591::getIn( uint8_t *d, int sz )
{
  return dev.recv( d, sz, addr );
}

int PCF8591::setOut( uint8_t v )
{
  uint8_t ou[2] = { cfg_reg, v };
  return dev.send( ou, 2, addr );
}



