#ifndef _OXC_PCF8591_H
#define _OXC_PCF8591_H

#include <oxc_base.h>

// inner regs: 1-byte addr

class PCF8591 {
  public:
   enum {
     def_addr     = 0x48,
     ain0         = 0x00,
     ain1         = 0x01,
     ain2         = 0x02,
     ain3         = 0x03,
     autoinc      = 0x04,
     mode_4in     = 0x00,
     mode_diffto3 = 0x10,
     mode_diff23  = 0x20,
     mode_diff4   = 0x30,
     out_en       = 0x40
   };

   PCF8591( I2C_HandleTypeDef &d_i2ch, uint8_t d_addr = def_addr )
     : i2ch( d_i2ch ), addr2( d_addr<<1 ) {};
   int  setMode( uint8_t mode );
   int  getIn( uint8_t *d, int sz );
   int  setOut( uint8_t v );
  private:
   I2C_HandleTypeDef &i2ch;
   uint8_t addr2;
   uint8_t cfg_reg = 0;
};

#endif
