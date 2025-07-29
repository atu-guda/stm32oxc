#ifndef _OXC_SPI_ADS1220_H
#define _OXC_SPI_ADS1220_H

// ADS1220 - ADC 4ch(mux), 24 bit, SPI, 20- 2000 Sps, PGA 1-4-128
// ADS1220_ADC_4ch_24bit_PGA_SPI.pdf

#include <optional>

// for debug
#include <oxc_auto.h>

#include <oxc_bitops.h>
#include <oxc_spi.h>

class ADS1220 {
  public:
   enum Params {
     n_cfgs   = 4,
     n_ch     = 4,
     bits     = 24,
     bits_out = 32,
     pkg_sz   = 2,
     scale    = 0x007FFFFF,
     vref     = 2108655, // default ref in uV
     max_wait = 100 // more than enough for 20 Sps (*2ms)
   };
   enum Cmds {
     CMD_PWRDN = 0x02,
     CMD_RESET = 0x06,
     CMD_START = 0x08,
     CMD_RDAT  = 0x10, // 0001 xxxx // unused? works w/o cmd?
     CMD_RREG  = 0x20, // 0010 rrnn
     CMD_WREG  = 0x40  // 0100 rrnn
   };
   enum Cfg0Bits {
     CFG0_PGA_BYPASS   =    1, // PGA_GAIN_* here, MUX_* here
     CFG0_PGA_GAIN_1   = 0x00,
     CFG0_PGA_GAIN_2   = 0x02,
     CFG0_PGA_GAIN_4   = 0x04,
     CFG0_PGA_GAIN_8   = 0x06,
     CFG0_PGA_GAIN_16  = 0x08,
     CFG0_PGA_GAIN_32  = 0x0A,
     CFG0_PGA_GAIN_64  = 0x0C,
     CFG0_PGA_GAIN_128 = 0x0E,
     CFG0_MUX_01       = 0x00,
     CFG0_MUX_02       = 0x10,
     CFG0_MUX_03       = 0x20,
     CFG0_MUX_12       = 0x30,
     CFG0_MUX_13       = 0x40,
     CFG0_MUX_23       = 0x50,
     CFG0_MUX_10       = 0x60,
     CFG0_MUX_32       = 0x70,
     CFG0_MUX_0S       = 0x80,
     CFG0_MUX_1S       = 0x90,
     CFG0_MUX_2S       = 0xA0,
     CFG0_MUX_3S       = 0xB0,
     CFG0_MUX_PN4      = 0xC0, // (V_refp-VRefn)/4
     CFG0_MUX_DS4      = 0xD0, // (AVdd-AVss)/4
     CFG0_MUX_MID      = 0xE0, // (AVdd+AVss)/2
   };

   enum Cfg1Bits {
     CFG1_BCS        =    1, // burn-out current source ON
     CFG1_TS         =    2, // Temperature source ON
     CFG1_1SHOT      =    0, // single-shot
     CFG1_CONT       =    4, // continuous mode
     CFG1_MODE_NORM  =    0, // normal tick mode
     CFG1_MODE_DUTY  = 0x08, // duty-cycle tick mode
     CFG1_MODE_TURBO = 0x10, // turbo tick mode
     CFG1_DR_20   = 0x00,
     CFG1_DR_45   = 0x20,
     CFG1_DR_90   = 0x40,
     CFG1_DR_175  = 0x60,
     CFG1_DR_330  = 0x80,
     CFG1_DR_600  = 0xA0,
     CFG1_DR_1000 = 0xC0,
   };

   enum Cfg2Bits {
     CFG2_IDAC_0    = 0x00, // in uA
     CFG2_IDAC_10   = 0x01,
     CFG2_IDAC_50   = 0x02,
     CFG2_IDAC_100  = 0x03,
     CFG2_IDAC_250  = 0x04,
     CFG2_IDAC_500  = 0x05,
     CFG2_IDAC_1000 = 0x06,
     CFG2_IDAC_1500 = 0x07,
     CFG2_PSW_OFF   = 0x00,
     CFG2_PSW_ON    = 0x08,
     CFG2_FILT_NO   = 0x00,
     CFG2_FILT_5060 = 0x10,
     CFG2_FILT_50   = 0x20,
     CFG2_FILT_60   = 0x30,
     CFG2_VFEF_INT  = 0x00,
     CFG2_VFEF_PN0  = 0x40,
     CFG2_VFEF_PN1  = 0x80,
     CFG2_VFEF_AVDD = 0xC0,
   };
   enum Cfg3Bits {
     CFG3_DRDYM_ON    = 0x01,
     CFG3_IDAC2_OFF   = 0x00 << 2,
     CFG3_IDAC2_AIN0  = 0x01 << 2,
     CFG3_IDAC2_AIN1  = 0x02 << 2,
     CFG3_IDAC2_AIN2  = 0x03 << 2,
     CFG3_IDAC2_AIN3  = 0x04 << 2,
     CFG3_IDAC2_REFP0 = 0x05 << 2,
     CFG3_IDAC2_REFN0 = 0x06 << 2,
     CFG3_IDAC1_OFF   = 0x00 << 5,
     CFG3_IDAC1_AIN0  = 0x01 << 5,
     CFG3_IDAC1_AIN1  = 0x02 << 5,
     CFG3_IDAC1_AIN2  = 0x03 << 5,
     CFG3_IDAC1_AIN3  = 0x04 << 5,
     CFG3_IDAC1_REFP0 = 0x05 << 5,
     CFG3_IDAC1_REFN0 = 0x06 << 5,
   };
   enum Masks {
     // masks
     DR_MASK       = 0xE0, // cfgs[1]
     PGA_GAIN_MASK = 0x0E, // cfgs[0]
     MUX_MASK      = 0xF0  // cfgs[0]
   };

   ADS1220( DevSPI &a_spi, PinsIn *a_ndrdy ) : spi_d( a_spi ), ndrdy( a_ndrdy ) {};
   constexpr static uint8_t mk_wr_cmd( uint8_t addr, uint8_t n=1 ) { return uint8_t(CMD_WREG | (addr << 2) | (n-1) ); };
   constexpr static uint8_t mk_rd_cmd( uint8_t addr, uint8_t n=1 ) { return uint8_t(CMD_RREG | (addr << 2) | (n-1) ); };
   uint8_t getCfg( uint8_t i ) const { return cfgs[i&0x03]; }
   const uint8_t* getCfgs() const { return cfgs; } // TODO: range?

   int writeReg( uint8_t addr, uint8_t val )
   {
     addr &= 0x03;
     const uint8_t b[pkg_sz]  { mk_wr_cmd(addr), val };
     return spi_d.send( b, sizeof(b) );
   }

   uint8_t readReg( uint8_t addr )
   {
     addr &= 0x03;
     const uint8_t bt[pkg_sz] { mk_rd_cmd(addr), 0xFF };
     uint8_t br[pkg_sz];
     spi_d.duplex( bt, br, sizeof(bt) );
     cfgs[addr] = br[1];
     return br[1];
   }

   int readAllRegs()
   {
     constexpr unsigned bsz = sizeof(cfgs)+1;
     const constexpr uint8_t bt[bsz] { mk_rd_cmd(0,sizeof(cfgs)), 0, 0, 0, 0 };
     uint8_t br[bsz];
     int rc = spi_d.duplex( bt, br, bsz );
     if( rc == bsz ) {
       for( unsigned i=0; i < sizeof(cfgs); ++i ) { // drop first
         cfgs[i] = br[i+1];
       }
     }
     return rc;
   }

   void init();
   int reset()  { return spi_d.send( CMD_RESET ); }
   int start()  { return spi_d.send( CMD_START ); }

   void PGA_en()
   {
     cfgs[0] &= ~CFG0_PGA_BYPASS;
     writeReg( 0, cfgs[0] );
   }

   void PGA_dis()
   {
     cfgs[0] |= CFG0_PGA_BYPASS;
     writeReg( 0, cfgs[0] );
   }

   void set_mux( Cfg0Bits mux_val )
   {
     cfgs[0] &= ~MUX_MASK;
     cfgs[0] |= mux_val;
     writeReg( 0, cfgs[0] );
   }

   void set_pga_gain( Cfg0Bits gain )
   {
     cfgs[0] &= ~PGA_GAIN_MASK;
     cfgs[0] |= gain;
     writeReg( 0, cfgs[0] );
   }

   void set_mode_continuous()
   {
     cfgs[1] |= CFG1_CONT;
     writeReg( 0, cfgs[1] );
   }

   void set_mode_1shot()
   {
     cfgs[1] &= ~CFG1_CONT;
     writeReg( 0, cfgs[1] );
   }

   void set_data_rate( Cfg1Bits dr )
   {
     cfgs[1] &= ~DR_MASK;
     cfgs[1] |= dr;
     writeReg( 0, cfgs[1] );
   }

   void read_config();
   std::optional<int32_t> read_nowait();
   std::optional<int32_t> read_wait();
   std::optional<int32_t> read_single();

  static const constexpr decltype(CFG0_MUX_0S) muxs_se[4] // single-ended mux params
    { ADS1220::CFG0_MUX_0S, ADS1220::CFG0_MUX_1S, ADS1220::CFG0_MUX_2S, ADS1220::CFG0_MUX_3S };

  protected:
   DevSPI &spi_d;
   PinsIn *ndrdy; // if nullptr, just wait
   uint8_t cfgs[n_cfgs] { CFG0_PGA_BYPASS | CFG0_PGA_GAIN_1 | CFG0_MUX_0S, CFG1_DR_20, CFG2_FILT_50, 0 };
};

#endif

// vim: path=.,/usr/share/stm32cube/inc
