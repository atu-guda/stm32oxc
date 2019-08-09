#ifndef _OXC_MCP4725_H
#define _OXC_MCP4725_H

//  MCP4725_DAC12bit_SOT-23-6.pdf

#include <oxc_i2c.h>


class MCP4725 : public I2CClient {
  public:
   enum {
     mcp4725_def_addr     = 0x60,
     mcp4725_sleep_1k     = 0x1000,
     mcp4725_sleep_50k    = 0x2000,
     mcp4725_sleep_500k   = 0x3000,
   };
   enum class Cmd {
     writeDAC    = 0x40,
     writeEEPROM = 0x60
   };

   MCP4725( DevI2C &a_dev, uint8_t d_addr = mcp4725_def_addr )
     : I2CClient( a_dev, d_addr ) {};
   void setOutFast( uint16_t v ) {
     send16_rev( v & 0x0FFF );
   };
   void sleep1k() {
     send16_rev( mcp4725_sleep_1k );
   };
   void sleep50k() {
     send16_rev( mcp4725_sleep_50k );
   };
   void sleep500k() {
     send16_rev( mcp4725_sleep_500k );
   };
   void setEEPROM( uint16_t v ) {
     send3b( v, Cmd::writeEEPROM  );
   };

   void send3b( uint16_t v, Cmd cmd );
  private:
};

#endif
