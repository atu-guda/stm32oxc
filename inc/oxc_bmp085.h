#ifndef _OXC_BMP085_H
#define _OXC_BMP085_H

#include <oxc_i2c.h>

class BMP085 : public I2CClient {
  public:
   enum {
     def_i2c_addr = 0x77,
     reg_calibr_start = 0xAA,
     n_calibr_data = 11,
     reg_cmd = 0xF4,
     reg_out = 0xF6,
     cmd_read_T = 0x2E,
     cmd_read_P0 = 0x34,
     cmd_read_P1 = 0x34 | 0x40,
     cmd_read_P2 = 0x34 | 0x80,
     cmd_read_P3 = 0x34 | 0xC0,
     cmd_read_P  = cmd_read_P3, // default: high_prec
     t_wait_T    = 5,
     t_wait_P    = 26
   };
   struct CalibrData {
     int16_t   ac1, ac2, ac3;
     uint16_t  ac4, ac5, ac6;
     int16_t   b1, b2, mb, mc, md;
   } __attribute__((packed));
   static_assert( sizeof(CalibrData) == n_calibr_data*sizeof(uint16_t), "Bad CalibrData size" );
   BMP085( DevI2C &a_dev, uint8_t d_addr = def_i2c_addr )
     : I2CClient( a_dev,  d_addr ) {};
   void init() { readCalibrData(); };
   bool readCalibrData();
   int  get_T_uncons( bool do_get = false );
   int  get_P_uncons( bool do_get = false );
   void getAllCalc( uint8_t a_oss );
   void calc();
   int  get_T10() { return t10; } // T10/10 = grad C
   int  get_P()   { return p;   }
   const int16_t* getCalibr() const { return (const int16_t*)(&calibr); }
  protected:
   CalibrData calibr;
   int32_t t_uncons = 0, t10 = 0;
   int32_t p_uncons = 0, p = 0;
   uint8_t oss = 0;
};

#endif
