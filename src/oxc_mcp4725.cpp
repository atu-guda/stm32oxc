#include <oxc_mcp4725.h>



void MCP4725::send3b( uint16_t v, Cmd cmd )
{
  uint8_t buf[4];
  buf[2] = (uint8_t)( v << 4 );
  buf[1] = (uint8_t)( v >> 4 );
  buf[0] = (uint8_t)( cmd );
  send( buf, 3 );
}




