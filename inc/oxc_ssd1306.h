#ifndef _OXC_SSD1306_H
#define _OXC_SSD1306_H

#include <oxc_pixbuf1v.h>

class SSD1306 {
  public:
   enum {
     BASE_ADDR = 0x3C,
     X_SZ = 128,
     Y_SZ = 64,
     I2C_TO  = 100,
     CMD_1 = 0x80,
     CMD_N = 0x00,
     DATA_1 = 0xC0,
     DATA_N = 0x40,
     CMD_MODE = 0x20,
     CMD_CONTRAST = 0x81,
     CMD_RAM = 0xA4, // default, output follows RAM
     CMD_FULLON = 0xA5, // output in ON, independent of RAM
     CMD_NOINVERSE = 0xA6,
     CMD_INVERSE = 0xA7,
     CMD_OFF = 0xAE,
     CMD_ON = 0xAF,
     MEM_SZ = ( X_SZ * Y_SZ / 8 )
   };
   SSD1306( I2C_HandleTypeDef &a_i2ch, uint8_t a_addr = BASE_ADDR )
     : i2ch( a_i2ch ), addr2( a_addr<<1 ) {};
   void init();
   void cmd1( uint8_t cmd );
   void cmd2( uint8_t cmd, uint8_t val );
   void data1( uint8_t d );

   void switch_on() { cmd1( CMD_ON ); };
   void switch_off() { cmd1( CMD_OFF ); };
   void contrast( uint8_t v ) { cmd2( CMD_CONTRAST, v ); };
   void full_on() { cmd1( CMD_FULLON ); };
   void on_ram() { cmd1( CMD_RAM ); };
   void no_inverse() { cmd1( CMD_NOINVERSE ); };
   void inverse() { cmd1( CMD_INVERSE ); };
   void mode_horisontal() { cmd2( CMD_MODE, 0x00 ); };
   void mode_vertical()   { cmd2( CMD_MODE, 0x01 ); };
   void mode_paged()      { cmd2( CMD_MODE, 0x02 ); };
   void out( PixBuf1V &pb );

  private:
   I2C_HandleTypeDef &i2ch;
   uint8_t addr2;
};

#endif

// vim: path=.,/usr/share/stm32cube/inc
