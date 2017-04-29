#ifndef _OXC_OXC_LSM303DLHC_MAG_H
#define _OXC_OXC_LSM303DLHC_MAG_H

#include <oxc_i2c.h>

// inner regs: 1-byte addr
// here only magnetometer

class LSM303DHLC_Mag {
  public:
   enum {
     def_addr     = 0x1E,
     reg_cra      = 0x00, // CR A (ODR + temp)
     reg_crb      = 0x01, // CR B (Gain)
     reg_mode     = 0x02, // Mode: (single, cont, sleep)
     reg_m_xh     = 0x03, // x mag Low
     reg_m_xl     = 0x04, // x mag High
     reg_m_yh     = 0x05, // y mag Low
     reg_m_yl     = 0x06, // y mag High
     reg_m_zh     = 0x07, // z mag Low
     reg_m_zl     = 0x08, // z mag High
     reg_sr       = 0x09, // Status: lock and drdy
     reg_temp_h   = 0x31, // Temperature: high
     reg_temp_l   = 0x32, // Temperature: low (upper 4 bit)
     cra_temp_en        =  0x80,
     cra_odr_0_75_Hz    =  0x00, // 0.75 Hz ODR
     cra_odr_1_5_Hz     =  0x04,
     cra_odr_3_0_Hz     =  0x08,
     cra_odr_7_5_Hz     =  0x0C,
     cra_odr_15_Hz      =  0x10,
     cra_odr_30_Hz      =  0x14,
     cra_odr_75_Hz      =  0x18,
     cra_odr_220_Hz     =  0x1C,
     crb_sens_1_3       =  0x20,
     crb_sens_1_9       =  0x40,
     crb_sens_2_5       =  0x60,
     crb_sens_4_0       =  0x80,
     crb_sens_4_7       =  0xA0,
     crb_sens_5_6       =  0xC0,
     crb_sens_8_1       =  0xE0
   };

   LSM303DHLC_Mag( DevI2C &a_dev, uint8_t d_addr = def_addr )
     : dev( a_dev ), addr( d_addr ) {};
   void setAddr( uint8_t d_addr ) { addr = d_addr; };
   uint8_t getAddr() const { return addr; }
   void resetDev() { dev.resetDev(); }
   bool init( uint8_t odr = cra_odr_75_Hz | cra_temp_en, uint8_t sens = crb_sens_1_9 );
   int16_t getReg( uint8_t reg ); // reg is 16-bit
   void    getRegs( uint8_t reg1, uint8_t n, int16_t *data );
   int16_t getMagX() { return getReg( reg_m_xl ); }
   int16_t getMagY() { return getReg( reg_m_yl ); }
   int16_t getMagZ() { return getReg( reg_m_zl ); }
   void    getMagAll( int16_t *mag ){ return getRegs( reg_m_xh, 3, mag ); }
   int16_t getTemp() { return getReg( reg_temp_h ); }
  private:
   DevI2C &dev;
   uint8_t addr;
};

#endif
