#ifndef _OXC_BMP280_H
#define _OXC_BMP280_H

#include <oxc_i2c.h>

class BMP280 : public I2CClient {
  public:
   enum {
     def_i2c_addr     = 0x77,
     reg_calibr_start = 0x88, // 0x88 -- 0xA1
     n_calibr_data    = ( (1 + 0xA1 - 0x88)/2 ), // 26 bytes, 16 [u]int16_t
     reg_id           = 0xD0,
     reg_rst          = 0xE0,
     reg_status       = 0xF3, // 0: update, 3: measuring
     reg_ctrl_meas    = 0xF4, // 0:1 - mode, 2:4 - osrs_p, 5:7 - osts_t
     reg_config       = 0xF5,
     reg_press_msb    = 0xF7,
     reg_press_lsb    = 0xF8,
     reg_press_xlsb   = 0xF9,
     reg_temp_msb     = 0xFA,
     reg_temp_lsb     = 0xFB,
     reg_temp_xlsb    = 0xFC,
     sz_data          = ( reg_temp_xlsb - reg_press_msb + 1 ),

     val_id           = 0x58,
     val_reset        = 0x6B,
     status_update    = 0x01,
     status_measuring = 0x08,

     ctrl_mode_sleep  = 0x00,
     ctrl_mode_force  = 0x01,
     ctrl_mode_force2 = 0x02,
     ctrl_mode_normal = 0x03,
     ctrl_osrs_t_skip = ( 0x00 << 2 ), // temperatue oversampling
     ctrl_osrs_t_x1   = ( 0x01 << 2 ),
     ctrl_osrs_t_x2   = ( 0x02 << 2 ),
     ctrl_osrs_t_x4   = ( 0x03 << 2 ),
     ctrl_osrs_t_x8   = ( 0x04 << 2 ),
     ctrl_osrs_t_xx   = ( 0x05 << 2 ),
     ctrl_osrs_t_x16  = ( 0x06 << 2 ),
     ctrl_osrs_p_skip = ( 0x00 << 5 ), // pressure oversampling
     ctrl_osrs_p_x1   = ( 0x01 << 5 ),
     ctrl_osrs_p_x2   = ( 0x02 << 5 ),
     ctrl_osrs_p_x4   = ( 0x03 << 5 ),
     ctrl_osrs_p_x8   = ( 0x04 << 5 ),
     ctrl_osrs_p_x16  = ( 0x05 << 5 ),
     ctrl_default     = ( ctrl_mode_normal | ctrl_osrs_t_x2 | ctrl_osrs_p_x16 ),

     cfg_spi_en       = 0x01,
     cfg_filter_off   = ( 0x00 << 2 ),
     cfg_filter_2     = ( 0x01 << 2 ),
     cfg_filter_4     = ( 0x02 << 2 ),
     cfg_filter_8     = ( 0x03 << 2 ),
     cfg_filter_16    = ( 0x04 << 2 ),
     cfg_sb_1         = ( 0x00 << 5 ), // standby time (~us)
     cfg_sb_63        = ( 0x01 << 5 ),
     cfg_sb_125       = ( 0x02 << 5 ),
     cfg_sb_250       = ( 0x03 << 5 ),
     cfg_sb_500       = ( 0x04 << 5 ),
     cfg_sb_1000      = ( 0x05 << 5 ),
     cfg_sb_2000      = ( 0x06 << 5 ),
     cfg_sb_4000      = ( 0x07 << 5 ),
     cfg_default      = ( cfg_filter_4 | cfg_sb_125 ),

   };

   struct CalibrData  {
     uint16_t  dig_t1; // 0x88
     int16_t   dig_t2; // 0x8A
     int16_t   dig_t3; // 0x8C
     uint16_t  dig_p1; // 0x8E
     int16_t   dig_p2, dig_p3, dig_p4, dig_p5, dig_p6, dig_p7, dig_p8, dig_p9; // 0x90
     int16_t   res1;
   } __attribute__((packed,aligned(__alignof__(uint16_t))));
   static_assert( sizeof(CalibrData) == n_calibr_data*sizeof(uint16_t), "Bad CalibrData size" );

   static inline int32_t cvt20bit( const uint8_t* d ) { return (d[2]>>4) | (d[1] << 4) | (d[0] << 12); }

   BMP280( DevI2C &a_dev, uint8_t d_addr = def_i2c_addr )
     : I2CClient( a_dev,  d_addr ) {};
   bool check_id();
   uint8_t getId() const { return id; };
   bool reset();
   void init() { readCalibrData(); };
   bool config( uint8_t cfg = cfg_default, uint8_t ctrl = ctrl_default );
   bool readCalibrData();
   bool readData();
   uint32_t  get_T_raw(  ) const { return cvt20bit( data + 3 ); }
   uint32_t  get_P_raw(  ) const { return cvt20bit( data     ); }
   // void getAllCalc( uint8_t a_oss );
   void calc();
   int32_t  get_T()   const { return t_100; } // T100/100 = grad C
   int32_t  get_P()   const { return p_fine;   }
   const CalibrData* getCalibr() const { return &calibr; }
   const uint8_t* getData() const { return data; }
  protected:
   CalibrData calibr;
   uint8_t data[sz_data];
   int32_t t_raw { 0 }, t_fine { 0 }, t_100 { 0 };
   int32_t p_raw { 0 }, p_fine { 0 };
   uint8_t oss { 0 };
   uint8_t id { 0 };
};

#endif
