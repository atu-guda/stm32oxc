#ifndef _OXC_MCP23017_H
#define _OXC_MCP23017_H

// i2c/MCP23017_IOexp_16bit_I2C.pdf

#include <oxc_i2c.h>

// inner regs: 1-byte addr

class MCP23017 : public I2CClient {
  public:
   enum {
     def_addr      = 0x20,
     // reg addresses with iocon.bank = 0 (default)
     reg_iodira    = 0x00, // 0 = in (def), 1 = out
     reg_iodirb    = 0x01,
     reg_iopola    = 0x02, // 0 = in = input (def), 1 - in = ! input
     reg_iopolb    = 0x03,
     reg_gpintena  = 0x04, // 0 = disable (def), 1 = enable INT on change
     reg_gpintenb  = 0x05,
     reg_defvala   = 0x06, // compare value for INT (if gpinten and intcon)
     reg_defvalb   = 0x07,
     reg_intcona   = 0x08, // 0 = cmp with defval (def), 1 = cmp with prev
     reg_intconb   = 0x09,
     reg_iocon     = 0x0A, // common cfg: 2 reg
     reg_iocon2    = 0x0B, // =^
     reg_gppua     = 0x0C, // 0 = no pull-up (def), 1 = pull-up
     reg_gppub     = 0x0D,
     reg_intfa     = 0x0E, // read: 0 - no, 1 - pin coused INT
     reg_intfb     = 0x0F,
     reg_intcapa   = 0x10, // read: captured pins during INT
     reg_intcapb   = 0x11,
     reg_gpioa     = 0x12, // read: values on port, write: pass to olat
     reg_gpiob     = 0x13,
     reg_olata     = 0x14,
     reg_olatb     = 0x15,
     //
     iocon_intpol  = 0x02, // INT out: 0 = active-low (def), 1 = active-high
     iocon_odr     = 0x04, // INT out mode: 0 = PP (def), 1 = OD (overrides intpol)
     iocon_haen    = 0x08, // only for SPI
     iocon_disslw  = 0x10, // SDA slew rate: 0 = en (def), 1 - disable
     iocon_seqop   = 0x20, // 0 = seq reg enabled (def), 1 - disabled
     iocon_mirror  = 0x40, // 0 = INTA and INTB independent (def), 1 = connected
     iocon_bank    = 0x80, // 0 = ports regs seq (def, see reg_xxxx), 1 - 2 blocks
   };

   MCP23017( DevI2C &a_dev, uint8_t d_addr = def_addr )
     : I2CClient( a_dev, d_addr ) {};
   // all functions assumes iocon_back = 0
   void cfg( uint8_t cfg_val ) { send_reg1_8bit( reg_iocon, cfg_val ); }
   void set_dir_a( uint8_t dir ) { send_reg1_8bit( reg_iodira, dir ); } // 1 = input
   void set_dir_b( uint8_t dir ) { send_reg1_8bit( reg_iodirb, dir ); }
   void set_dir( uint16_t dir )  { send_reg1_8bit( reg_iodira, dir & 0xFF );  send_reg1_8bit( reg_iodirb, dir >> 8 ); }
   void set_inv_in_a( uint8_t iv ) { send_reg1_8bit( reg_iopola, iv ); } // 1 = inverse
   void set_inv_in_b( uint8_t iv ) { send_reg1_8bit( reg_iopolb, iv ); }
   void set_int_en_a( uint8_t ie ) { send_reg1_8bit( reg_gpintena, ie ); } // 1 = int enable
   void set_int_en_b( uint8_t ie ) { send_reg1_8bit( reg_gpintenb, ie ); }
   void set_defval_a( uint8_t dv ) { send_reg1_8bit( reg_defvala, dv ); }
   void set_defval_b( uint8_t dv ) { send_reg1_8bit( reg_defvalb, dv ); }
   void set_intcon_a( uint8_t ic ) { send_reg1_8bit( reg_intcona, ic ); }
   void set_intcon_b( uint8_t ic ) { send_reg1_8bit( reg_intconb, ic ); }
   void set_pullup_a( uint8_t pu ) { send_reg1_8bit( reg_gppua, pu ); }
   void set_pullup_b( uint8_t pu ) { send_reg1_8bit( reg_gppub, pu ); }
   uint8_t get_intflag_a() { return recv_reg1_8bit( reg_intfa ); }
   uint8_t get_intflag_b() { return recv_reg1_8bit( reg_intfb ); }
   uint8_t get_intcp_a() { return recv_reg1_8bit( reg_intcapa ); }
   uint8_t get_intcp_b() { return recv_reg1_8bit( reg_intcapb ); }
   uint8_t get_a() { return recv_reg1_8bit( reg_gpioa ); }
   uint8_t get_b() { return recv_reg1_8bit( reg_gpiob ); }
   uint16_t get() { return recv_reg1_8bit( reg_gpioa ) | recv_reg1_8bit( reg_gpiob ) << 8 ; }
   void set_a( uint8_t v ) { send_reg1_8bit( reg_olata, v ); }
   void set_b( uint8_t v ) { send_reg1_8bit( reg_olatb, v ); }
   void set( uint16_t v ) { send_reg1_8bit( reg_olata, v & 0xFF ); send_reg1_8bit( reg_olatb, v >> 8 ); }
  private:
};

#endif
