#ifndef _OXC_PCD8544_H
#define _OXC_PCD8544_H

// PCD8544 - LCD controller, used in Nokia 3110 5110 LCD screens
// Used one-way SPI + RST, D/C, (-LED: here not handled)

#include <oxc_spi.h>
#include <oxc_pixbuf1v.h>

class PCD8544 {
  public:
   enum {
     X_SZ             = 84,
     Y_SZ             = 48,
     FUNC_SET         = 0x20,
     FUNC_SET_PD      = 0x04, // 1 = power down
     FUNC_SET_VMODE   = 0x02, // 1 = Vertical mode
     FUNC_SET_XCMD    = 0x01, // 1 = eXtendec cmds
     FUNC_XCMD        = FUNC_SET | FUNC_SET_XCMD,
     FUNC_NCMD        = FUNC_SET, // normal (base) cnd
     // base commands
     CMD_CFG          = 0x08,
     CMD_CFG_D        = 0x04, // DE: 00=Empty, 10=Normal, 01=FullOn, 11 = Inverse
     CMD_CFG_E        = 0x01,
     CMD_CFG_DFLT     = CMD_CFG | CMD_CFG_D,
     CMD_Y_POS        = 0x40, // | 3 bit y addr
     CMD_X_POS        = 0x80, // | 7 bit x addr
     // extended cmds
     XCMD_TCX         = 0x04, // + 2 bit on TCx
     XCMD_TCX_DFLT    = 0x06, //   defalut TCx
     XCMD_BIAS        = 0x10, // + 3 bit on BSx
     XCMD_BIAS_DFLT   = 0x13, //   default BIAS
     XCMD_VOP         = 0x80, // + 7 bit on Vop
     XCMD_VOP_DFLT    = 0xC8, //   default Vop
     MEM_SZ = ( X_SZ * Y_SZ / 8 )
   };
   PCD8544( DevSPI &a_spi, PinOut &a_rst, PinOut &a_dc )
     : spi_d( a_spi ),  rst( a_rst ), dc( a_dc ) {};
   int init();
   void reset() { rst.reset(); delay_bad_mcs( 10 ); rst.set(); }
   int cmd( const uint8_t *cmds, uint16_t n );
   int cmd( uint8_t acmd ) { return cmd( &acmd, 1 ); };
   int xcmd( uint8_t acmd ); // send extended cmd + switch mode +-
   int ncmd( uint8_t acmd ); // send normal cmd + switch mode -
   int data( const uint8_t *d, uint16_t n );

   int switch_on()  { return cmd( FUNC_NCMD ); };
   int switch_off() { return cmd( FUNC_NCMD | FUNC_SET_PD ); };
   int contrast( uint8_t v ) { return xcmd( XCMD_VOP | ( v & 0x7F ) ); };
   int full_on()    { return ncmd( CMD_CFG | CMD_CFG_E ); };
   int no_inverse() { return ncmd( CMD_CFG_DFLT ); };
   int inverse()    { return ncmd( CMD_CFG | CMD_CFG_D | CMD_CFG_E ); };
   int out( PixBuf1V &pb );

  protected:
   DevSPI &spi_d;
   PinOut &rst;
   PinOut &dc;
};

#endif

// vim: path=.,/usr/share/stm32cube/inc
