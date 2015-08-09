#include <oxc_pcf8591.h>


int PCF8591::setMode( uint8_t mode )
{
  cfg_reg = mode;
  return send( cfg_reg );
}

int PCF8591::getIn( uint8_t *d, int sz )
{
  return recv( d, sz );
}

int PCF8591::setOut( uint8_t v )
{
  uint8_t ou[2] = { cfg_reg, v };
  return send( ou, 2 );
}



