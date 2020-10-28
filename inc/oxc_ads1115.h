#ifndef _OXC_ADS1115_H
#define _OXC_ADS1115_H

// 16-bit 4 input xx ADC I2C
// work with ADS1015 - 12 bit ADC - only different timings

#include <oxc_i2c.h>

// inner regs: 1-byte addr, 2 byte data

class ADS1115 : public I2CClient {
  public:
   enum {
     n_ch_max        = 4,
     def_addr        = 0x48,
     reg_data        = 0x00,
     reg_cfg         = 0x01,
     reg_alarm_low   = 0x02,
     reg_alarm_high  = 0x03,
     cfg_default     = 0x8583,
     cfg_os          = 0x8000, // write: start shot, read: 0: now converiong, 1: now not converting
     cfg_in_dif_0_1  = 0x0000, // default
     cfg_in_dif_0_3  = 0x1000,
     cfg_in_dif_1_3  = 0x2000,
     cfg_in_dif_2_3  = 0x3000,
     cfg_in_0        = 0x4000, // sinle-ended
     cfg_in_1        = 0x5000,
     cfg_in_2        = 0x6000,
     cfg_in_3        = 0x7000,
     cfg_in_mask     = 0x7000,
     cfg_pga_6144    = 0x0000, // PGA: input is +- 6.144 V
     cfg_pga_4096    = 0x0200, //
     cfg_pga_2048    = 0x0400, // default
     cfg_pga_1024    = 0x0600,
     cfg_pga_0512    = 0x0800,
     cfg_pga_0256    = 0x0A00,
     cfg_pga_0256a   = 0x0C00,
     cfg_pga_0256b   = 0x0E00,
     cfg_pga_mask    = 0x0E00,
     cfg_oneShot     = 0x0100,

     // for ADS1115
     cfg_rate_008    = 0x0000,
     cfg_rate_016    = 0x0020,
     cfg_rate_032    = 0x0040,
     cfg_rate_064    = 0x0060,
     cfg_rate_128    = 0x0080,
     cfg_rate_250    = 0x00A0,
     cfg_rate_475    = 0x00C0,
     cfg_rate_860    = 0x00E0,
     cfg_rate_mask   = 0x00E0,

     // for ADS1015
     cfg_1015_rate_0128    = 0x0000,
     cfg_1015_rate_0250    = 0x0020,
     cfg_1015_rate_0490    = 0x0040,
     cfg_1015_rate_0920    = 0x0060,
     cfg_1015_rate_1600    = 0x0080,
     cfg_1015_rate_2400    = 0x00A0,
     cfg_1015_rate_3300    = 0x00C0,
     cfg_1015_rate_3301    = 0x00E0, // really 3300

     cfg_comp_window = 0x0010,
     cfg_comp_pol    = 0x0008, // defaut is 0 - active low
     cfg_comp_latch  = 0x0004, // defaut is 0 - active low
     cfg_comp_1      = 0x0000,
     cfg_comp_2      = 0x0001,
     cfg_comp_4      = 0x0002,
     cfg_comp_no     = 0x0003  // default
   };

   ADS1115( DevI2C &a_dev, uint8_t d_addr = def_addr )
     : I2CClient( a_dev, d_addr ) {};
   bool writeCurrCfg() { return writeReg( reg_cfg, cfg_val ); };
   bool setCfg( uint16_t cv )   { cfg_val  = cv; return writeCurrCfg(); };
   bool addToCfg( uint16_t cv ) { cfg_val |= cv; return writeCurrCfg(); };
   bool setDefault()   { cfg_val  = cfg_default; return writeCurrCfg(); };
   uint16_t getStoredCfg() const { return cfg_val; };
   uint16_t getDeviceCfg() { cfg_val =  readReg( reg_cfg ); return cfg_val; }
   int16_t getOneShot();
   int16_t getContValue() { return readReg( reg_data ); }
   int getOneShotNch( uint8_t s_ch, uint8_t e_ch, int16_t *d );
   bool startCont() { cfg_val &= ~cfg_oneShot; return writeCurrCfg(); };
   bool stopCont()  { cfg_val |=  cfg_oneShot; return writeCurrCfg(); };
   int getScale_mV() const; // full scale per 32768
   bool setAlarmLow( uint16_t v ) { return writeReg( reg_alarm_low, v ); };
   bool setAlarmHigh( uint16_t v ) { return writeReg( reg_alarm_high, v ); };
  protected:
   uint16_t readReg( uint8_t reg );
   bool writeReg( uint8_t reg, uint16_t val );
   uint16_t cfg_val = cfg_default;
   static const int scale_mv[8];
   // static int coeff_nv[] = { 187500, 125000, 62500, 31250, 15625, 7813,  7813  7813 };
};

#endif
