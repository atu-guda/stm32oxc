#ifndef _OXC_SPI_DAC8563_H
#define _OXC_SPI_DAC8563_H

#include <oxc_spi.h>
//#include <oxc_debug1.h> // for dump8
//#include <oxc_outstream.h>

// 1 packet = 24 bit
// 23-24 - NC
// 19-21 - CMD
// 16-18 - ADDR
// 00-15 - data

// To control LDAC You need aux GPIO output line.

class DevSPI_DAC8563 {
  public:
   enum Props {
     bits = 16,
     maxval = 0xFFFF
   };
   enum CmdAddr {
     PKT_LEN   =  3,
     ADDR_A    =  0,
     ADDR_B    =  1,
     ADDR_GAIN =  2,
     ADDR_AB   =  7,
     // pure cmd
     CMD_SET   = 0 << 3,
     CMD_UPD   = 1 << 3,
     CMD_SETUA = 2 << 3, // set and update all
     CMD_SETU  = 3 << 3, // set and update given
     CMD_PWR   = 4 << 3, // power commands
     CMD_RST   = 5 << 3,
     CMD_LDAC  = 6 << 3,
     CMD_IREF  = 7 << 3,
     // cmd | addr
     CMD_SET_A     = CMD_SET   | ADDR_A,
     CMD_SET_B     = CMD_SET   | ADDR_B,
     CMD_SET_AB    = CMD_SET   | ADDR_AB,
     CMD_UPD_A     = CMD_UPD   | ADDR_A,
     CMD_UPD_B     = CMD_UPD   | ADDR_B,
     CMD_UPD_AB    = CMD_UPD   | ADDR_AB,
     CMD_SETUA_A   = CMD_SETUA | ADDR_A,
     CMD_SETUA_B   = CMD_SETUA | ADDR_B,
     CMD_SETUA_AB  = CMD_SETUA | ADDR_AB,
     CMD_SETU_A    = CMD_SETU  | ADDR_A,
     CMD_SETU_B    = CMD_SETU  | ADDR_B,
     CMD_SETU_AB   = CMD_SETU  | ADDR_AB,
     CMD_GAIN      = CMD_SET   | ADDR_GAIN, // + 2bit in data
   };

   DevSPI_DAC8563( DevSPI &a_spi )
     : spi( a_spi ) {};

   void init();

   int write( uint8_t cmd_addr, uint16_t val ) {
     //std_out << "# ca= " << HexInt8( cmd_addr ) << " v= " << HexInt16( val ) << NL;
     const uint8_t b[PKT_LEN] = { cmd_addr, uint8_t(val>>8), uint8_t(val) };
     // dump8( b, PKT_LEN );
     return spi.send( b, PKT_LEN );
   }

   int setu( uint8_t ch, uint16_t val ) { return write( ch_setu_cmd[ ch & 3 ], val ); };
   int setu_a(  uint16_t val ) { return write( CMD_SETU_A,  val); };
   int setu_b(  uint16_t val ) { return write( CMD_SETU_B,  val); };
   int setu_ab( uint16_t val ) { return write( CMD_SETU_AB, val); };

   int reset_ab() { return spi.send( dat_reset_ab,  PKT_LEN ); }
   int reset()    { return spi.send( dat_reset,     PKT_LEN ); }
   int iref_off() { return spi.send( dat_iref_off,  PKT_LEN ); }
   int iref_on()  { return spi.send( dat_iref_on,   PKT_LEN ); }
   int ldac_ab()  { return spi.send( dat_ldac_ab,   PKT_LEN ); }
   int ldac_xb()  { return spi.send( dat_ldac_xb,   PKT_LEN ); }
   int ldac_ax()  { return spi.send( dat_ldac_ax,   PKT_LEN ); }
   int ldac_xx()  { return spi.send( dat_ldac_xx,   PKT_LEN ); }
   int pwrup_a()  { return spi.send( dat_pwrup_a,   PKT_LEN ); }
   int pwrup_b()  { return spi.send( dat_pwrup_b,   PKT_LEN ); }
   int pwrup_ab() { return spi.send( dat_pwrup_ab,  PKT_LEN ); }
   int pwrdn_a()  { return spi.send( dat_pwrdn_a,   PKT_LEN ); }
   int pwrdn_b()  { return spi.send( dat_pwrdn_b,   PKT_LEN ); }
   int pwrdn_ab() { return spi.send( dat_pwrdn_ab,  PKT_LEN ); }
   int gain_22()  { return spi.send( dat_gain_22,   PKT_LEN ); }
   int gain_21()  { return spi.send( dat_gain_21,   PKT_LEN ); }
   int gain_12()  { return spi.send( dat_gain_12,   PKT_LEN ); }
   int gain_11()  { return spi.send( dat_gain_11,   PKT_LEN ); }

  protected:
   DevSPI &spi;
   static const constexpr uint8_t ch_setu_cmd[4] { CMD_SETU_A, CMD_SETU_B, CMD_SETU_AB, CMD_SETU_AB }; // A, B, AB, reserve
   static const constexpr uint8_t dat_reset_ab[PKT_LEN]    { CMD_RST,   0x00, 0x00 };
   static const constexpr uint8_t dat_reset[PKT_LEN]       { CMD_RST,   0x00, 0x01 };
   static const constexpr uint8_t dat_iref_off[PKT_LEN]    { CMD_IREF,  0x00, 0x00 };
   static const constexpr uint8_t dat_iref_on[PKT_LEN]     { CMD_IREF,  0x00, 0x01 };
   static const constexpr uint8_t dat_ldac_ab[PKT_LEN]     { CMD_LDAC,  0x00, 0x00 };
   static const constexpr uint8_t dat_ldac_xb[PKT_LEN]     { CMD_LDAC,  0x00, 0x01 };
   static const constexpr uint8_t dat_ldac_ax[PKT_LEN]     { CMD_LDAC,  0x00, 0x02 };
   static const constexpr uint8_t dat_ldac_xx[PKT_LEN]     { CMD_LDAC,  0x00, 0x03 };
   static const constexpr uint8_t dat_pwrup_a[PKT_LEN]     { CMD_PWR,   0x00, 0x01 };
   static const constexpr uint8_t dat_pwrup_b[PKT_LEN]     { CMD_PWR,   0x00, 0x02 };
   static const constexpr uint8_t dat_pwrup_ab[PKT_LEN]    { CMD_PWR,   0x00, 0x03 };
   static const constexpr uint8_t dat_pwrdn_a[PKT_LEN]     { CMD_PWR,   0x00, 0x19 }; // High-Z
   static const constexpr uint8_t dat_pwrdn_b[PKT_LEN]     { CMD_PWR,   0x00, 0x1A };
   static const constexpr uint8_t dat_pwrdn_ab[PKT_LEN]    { CMD_PWR,   0x00, 0x1B };
   static const constexpr uint8_t dat_gain_22[PKT_LEN]     { CMD_GAIN,  0x00, 0x00 };
   static const constexpr uint8_t dat_gain_12[PKT_LEN]     { CMD_GAIN,  0x00, 0x01 };
   static const constexpr uint8_t dat_gain_21[PKT_LEN]     { CMD_GAIN,  0x00, 0x02 };
   static const constexpr uint8_t dat_gain_11[PKT_LEN]     { CMD_GAIN,  0x00, 0x03 };
};


#endif

