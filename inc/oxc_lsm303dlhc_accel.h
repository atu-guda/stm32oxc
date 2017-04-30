#ifndef _OXC_OXC_LSM303DLHC_ACCEL_H
#define _OXC_OXC_LSM303DLHC_ACCEL_H

#include <oxc_i2c.h>

// inner regs: 1-byte addr, need to set bit 7 to continous access
// rehe only accelerometer, magnetometer in another file

class LSM303DHLC_Accel : public I2CClient {
  public:
   enum {
     def_addr     = 0x19,
     reg_id       = 0x0F, // who am i
     id_resp      = 0x33, // responce to id
     reg_ctl1     = 0x20, // CTRL_REG1_A
     reg_ctl2     = 0x21, // CTRL_REG2_A
     reg_ctl3     = 0x22, // CTRL_REG3_A
     reg_ctl4     = 0x23, // CTRL_REG4_A
     reg_ctl5     = 0x24, // CTRL_REG5_A
     reg_ctl6     = 0x25, // CTRL_REG6_A
     reg_ref      = 0x26, // Reference register
     reg_status   = 0x27, // Status register
     reg_a_xl     = 0x28, // x accel Low
     reg_a_xh     = 0x29, // x accel High
     reg_a_yl     = 0x2A, // y accel Low
     reg_a_yh     = 0x2B, // y accel High
     reg_a_zl     = 0x2C, // z accel Low
     reg_a_zh     = 0x2D, // z accel High
     reg_fifo_ctl = 0x2E, // Fifo control Register
     reg_fifo_src = 0x2F, // Fifo source Register
     reg_int1_cfg       =  0x30, // Interrupt 1 configuration Register acceleration
     reg_int1_source    =  0x31, // Interrupt 1 source Register acceleration
     reg_int1_ths       =  0x32, // Interrupt 1 Threshold register
     reg_int1_duration  =  0x33, // Interrupt 1 DURATION register
     reg_int2_cfg       =  0x34, // Interrupt 2 configuration Register
     reg_int2_source    =  0x35, // Interrupt 2 source Register
     reg_int2_ths       =  0x36, // Interrupt 2 Threshold register
     reg_int2_duration  =  0x37, // Interrupt 2 DURATION register
     reg_click_cfg      =  0x38, // Click configuration Register
     reg_click_source   =  0x39, // Click 2 source Register
     reg_click_ths      =  0x3A, // Click 2 Threshold register
     reg_time_limit     =  0x3B, // Time Limit Register
     reg_time_latency   =  0x3C, // Time Latency Register
     reg_time_window    =  0x3D, // Time window register
   };
   enum Ctl1_val { // for ctl1 reg
     power_mode_normal = 0x00,
     power_mode_low    = 0x08,
     odr_1_Hz          = 0x10,  //* Output Data Rate
     odr_10_Hz         = 0x20,
     odr_25_Hz         = 0x30,
     odr_50_Hz         = 0x40,
     odr_100_Hz        = 0x50,
     odr_200_Hz        = 0x60,
     odr_400_Hz        = 0x70,
     odr_1620_Hz_lp    = 0x80,  //* only in Low Power Mode */
     odr_1344_Hz       = 0x90,  //* 1344 Hz in Normal mode and 5376 Hz in Low Power Mode
     x_enable          = 0x01,
     y_enable          = 0x02,
     z_enable          = 0x04,
     axes_enable       = 0x07,
     ctl1_def          = power_mode_normal | odr_100_Hz | axes_enable
   };
   enum Ctl2_val { // for ctl2 reg (filters)
     hpis1_enable      = 0x01,
     hpis2_enable      = 0x02,
     hpclick_enable    = 0x04,
     fds_enable        = 0x08,
     hpm_normal_rst    = 0x00,
     hpm_normal        = 0x80,
     hpm_ref           = 0x40,
     hpm_autoreset     = 0xC0,
     freq_0            = 0x00,
     freq_1            = 0x10,
     freq_2            = 0x20,
     freq_3            = 0x30,
     ctl2_def = hpm_normal | freq_0
   };
   enum Ctl4_val { // for ctl4 reg (scale...)
     bdu_enable        = 0x80, // block data update
     ble_enable        = 0x40, // MSB = low addr
     scale_2g          = 0x00,
     scale_4g          = 0x10,
     scale_8g          = 0x20,
     scale_16g         = 0x30,
     hr_enable         = 0x08, // high resolution
     ctl4_def          = scale_2g
   };

   LSM303DHLC_Accel( DevI2C &a_dev, uint8_t d_addr = def_addr )
     : I2CClient( a_dev, d_addr ) {};
   bool check_id();
   bool init( Ctl4_val c4 = ctl4_def, Ctl2_val c2 = ctl2_def, Ctl1_val c1 = ctl1_def );
   void rebootMem();
   // void setAccScale( ACC_scale accs ) { send_reg1( , accs ); }
   int16_t getReg( uint8_t reg ); // reg is 16-bit
   void    getRegs( uint8_t reg1, uint8_t n, int16_t *data );
   int16_t getAccX() { return getReg( reg_a_xl ); }
   int16_t getAccY() { return getReg( reg_a_yl ); }
   int16_t getAccZ() { return getReg( reg_a_zl ); }
   void    getAccAll( int16_t *acc ){ return getRegs( reg_a_xl, 3, acc ); }
  protected:
};

#endif
