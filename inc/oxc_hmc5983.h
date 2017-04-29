#ifndef _OXC_OXC_HMC5983_H
#define _OXC_OXC_HMC5983_H

// HMC5983 3 Axis Compass HMC5983_3_Axis_Compass_IC.pdf

#include <oxc_i2c.h>

// inner regs: 1-byte addr

class HMC5983 {
  public:
   enum RegNums {
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
     reg_id_a     = 0x0A, // 0x48 = 'H'
     reg_id_b     = 0x0B, // 0x34 = '4'
     reg_id_c     = 0x0C, // 0x33 = '3'
     reg_temp_h   = 0x31, // Temperature: high
     reg_temp_l   = 0x32  // Temperature: low (upper 4 bit)
   };
   enum CRA {
     // CRA bits
     cra_temp_en        =  0x80,
     cra_odr_0_75_Hz    =  0x00, // 0.75 Hz SPS
     cra_odr_1_5_Hz     =  0x04,
     cra_odr_3_0_Hz     =  0x08,
     cra_odr_7_5_Hz     =  0x0C,
     cra_odr_15_Hz      =  0x10,
     cra_odr_30_Hz      =  0x14,
     cra_odr_75_Hz      =  0x18,
     cra_odr_220_Hz     =  0x1C,
     cra_meas_normal    =  0x00,
     cra_meas_plusbias  =  0x01,
     cra_meas_minusbias =  0x02,
     cra_meas_tonly     =  0x03
   };
   enum CRB {
     // CRB bits: scale (Ga) /gain
     crb_scale_0_88     =  0x00, // 0 1370 LSb/Ga,  730 mcGa/LSb
     crb_scale_1_3      =  0x20, // 1 1090 LSb/Ga,  920 mcGa/LSb (def)
     crb_scale_1_9      =  0x40, // 2  820 LSb/Ga, 1220 mcGa/LSb
     crb_scale_2_5      =  0x60, // 3  660 LSb/Ga, 1520 mcGa/LSb
     crb_scale_4_0      =  0x80, // 4  440 LSb/Ga, 2270 mcGa/LSb
     crb_scale_4_7      =  0xA0, // 5  390 LSb/Ga, 2560 mcGa/LSb
     crb_scale_5_6      =  0xC0, // 6  330 LSb/Ga, 3030 mcGa/LSb
     crb_scale_8_1      =  0xE0  // 7  230 LSb/Ga, 4350 mcGa/LSb
   };
   enum Scales {
     // scale indexs
     scale_0_88         =  0, // 0 1370 LSb/Ga,  730 mcGa/LSb
     scale_1_3          =  1, // 1 1090 LSb/Ga,  920 mcGa/LSb (def)
     scale_1_9          =  2, // 2  820 LSb/Ga, 1220 mcGa/LSb
     scale_2_5          =  3, // 3  660 LSb/Ga, 1520 mcGa/LSb
     scale_4_0          =  4, // 4  440 LSb/Ga, 2270 mcGa/LSb
     scale_4_7          =  5, // 5  390 LSb/Ga, 2560 mcGa/LSb
     scale_5_6          =  6, // 6  330 LSb/Ga, 3030 mcGa/LSb
     scale_8_1          =  7, // 7  230 LSb/Ga, 4350 mcGa/LSb
     n_scales           =  8, // 8 number of above
     max_loops          = 1000 // for read1
   };

   enum ModeBits {
     mode_high_i2c      = 0x80, // enable high-speed I2C 3400 kHz
     mode_low_power     = 0x20, // low power mode, 0.75 Hz, no aver
     mode_idle          = 0x03, // idle (def)
     mode_idle1         = 0x02, // idle
     mode_single        = 0x01, // single conversion
     mode_cont          = 0x00  // serial conversion
   };
   enum StatusBits {
     status_rdy         = 0x01, // ready bit
     status_lock        = 0x02, // locked while read!!!
     status_dow         = 0x10  // Data Over Written
   };

   HMC5983( DevI2C &a_dev, uint8_t d_addr = def_addr )
     : dev( a_dev ), addr( d_addr ) {};
   void setAddr( uint8_t d_addr ) { addr = d_addr; };
   uint8_t getAddr() const { return addr; }
   void resetDev() { dev.resetDev(); }
   int  getErr() const { return dev.getErr(); };
   bool init( CRA odr = cra_odr_75_Hz, Scales scale = scale_1_9 );
   bool read1( int32_t wait_ms = -1 ); // per 1  wait loop, def=no wait, max_loops
   bool startAuto();
   bool readNextAuto(int32_t wait_ms = -1 );
   bool stopAuto();
   int16_t getReg( uint8_t reg ); // reg is 16-bit
   bool    getRegs( uint8_t reg1, uint8_t n, int16_t *data );
   static constexpr uint16_t getNScales() { return n_scales; }
   int16_t getMagX() const { return regsXYZ[0]; }
   int16_t getMagY() const { return regsXYZ[1]; }
   int16_t getMagZ() const { return regsXYZ[2]; }
   const int16_t*  getMagAll( int16_t *mag ) const { return regsXYZ; }
   int32_t getMagXmcGa() const { return valsXYZ[0]; };
   int32_t getMagYmcGa() const { return valsXYZ[1]; };
   int32_t getMagZmcGa() const { return valsXYZ[2]; };
   const int32_t*  getMagAllmcGa() const{ return valsXYZ; };
   int16_t getTemp() { int16_t v = getReg( reg_temp_h );  v /= 128; v += 25; return v; } // TODO: calibrate!
   uint32_t get_mcGa_LSb() const { return mcGa_LSb; }
  private:
   DevI2C &dev;
   uint8_t addr;
   bool wait_read( int32_t wait_ms  = -1 );
   //
   int32_t mcGa_LSb = 0;
   static const int32_t mcGa_LSbs[n_scales];
   static const char    scale_bits[n_scales];
   int16_t regsXYZ[3] { 0, 0, 0 };
   int32_t valsXYZ[3] { 0, 0, 0 };
};

#endif
