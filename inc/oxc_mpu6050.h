#ifndef _OXC_MPU6050_H
#define _OXC_MPU6050_H

// Accel, Gyro I2C: PS-MPU-6000A-00v3.4.pdf

#include <oxc_i2c.h>

// inner regs: 1-byte addr

class MPU6050 : public I2CClient {
  public:
   enum {
     mpu6050_def_addr     = 0x68,
     mpu6050_alldata_sz   = 7,
     mpu6050_reg_smplrt   = 0x19, // sample rate
     mpu6050_reg_cfg      = 0x1A, // config; [0:2]-DLPF, [3:5]-EXtSync
     mpu6050_reg_cfg_gyro = 0x1B, // [3:4]-FS_SEL
     mpu6050_reg_cfg_acc  = 0x1C, // [3:4]-FS_SEL, 5-ZA_ST, 6-YA_ST, 7-XA_ST
     mpu6050_reg_a_xh     = 0x3B, // x accel High
     mpu6050_reg_a_xl     = 0x3C, // x accel Low
     mpu6050_reg_a_yh     = 0x3D, // y accel High
     mpu6050_reg_a_yl     = 0x3E, // y accel Low
     mpu6050_reg_a_zh     = 0x3F, // z accel High
     mpu6050_reg_a_zl     = 0x40, // z accel Low
     mpu6050_reg_temph    = 0x41, // Temp High : temperatue returns C*100
     mpu6050_reg_templ    = 0x42, // Temp Low
     mpu6050_reg_g_xh     = 0x43, // x gyro High
     mpu6050_reg_g_xl     = 0x44, // x gyro Low
     mpu6050_reg_g_yh     = 0x45, // y gyro High
     mpu6050_reg_g_yl     = 0x46, // y gyro Low
     mpu6050_reg_g_zh     = 0x47, // z gyro High
     mpu6050_reg_g_zl     = 0x48, // z gyro Low
     mpu6050_reg_pwr1     = 0x6B, // power mgmt 1: [0:2]-Clock [3]-TempDis, [5]-Cycle, [6]-Sleep [7]-Rst
     mpu6050_reg_pwr2     = 0x6C, // power mgmt 2
     mpu6050_reg_id       = 0x75  // who am i
   };
   enum DLP_BW {
     bw_260 = 0, bw_184 = 1, bw_94 = 2, bw_44 = 3, bw_21 = 4, bw_10 = 5, bw_5 = 6
   };
   enum ACC_scale {
     accs_2g = 0, accs_4g = 0x08, accs_8g = 0x10, accs_16g = 0x18,
     accs_test_z = 0x20, accs_test_y = 0x40, accs_test_x = 0x80
   };
   enum Gyro_scale {
     gyros_250 = 0, gyros_500 = 0x08, gyros_1000 = 0x10, gyros_2000 = 0x18,
     gyros_test_z = 0x20, gyros_test_y = 0x40, gyros_test_x = 0x80
   };
   enum PLL_source {
     pll_internal = 0, pll_gyro_x = 1, pll_gyro_y = 2, pll_gyro_z = 3,
     pll_ext32k = 4,   pll_ext19m = 5, pll_stop = 7,   pll_sleep = 0x40
   };

   MPU6050( DevI2C &a_dev, uint8_t d_addr = mpu6050_def_addr )
     : I2CClient( a_dev, d_addr ) {};
   void init();
   void sleep() {  send_reg1_8bit( mpu6050_reg_pwr1, pll_sleep ); }
   void wake( PLL_source sou ){ send_reg1_8bit( mpu6050_reg_pwr1, sou ); }
   void setDLP( DLP_BW dlp_bw ) { send_reg1_8bit( mpu6050_reg_cfg, dlp_bw ); }
   void setAccScale( ACC_scale accs ) { send_reg1_8bit( mpu6050_reg_cfg_acc, accs ); }
   void setGyroScale( Gyro_scale gyros ) { send_reg1_8bit( mpu6050_reg_cfg_gyro, gyros ); }
   static int fixTemp( int v ) { return v*10/34 + 3653; };
   int16_t getReg( uint8_t reg )
     { return recv_reg1_16bit_rev( reg, 0 ); };
   int    getRegs( uint8_t reg1, uint8_t n, int16_t *data )
     { return recv_reg1_16bit_n_rev( reg1, (uint16_t*)data, n ); }
   int16_t getAccX() { return getReg( mpu6050_reg_a_xh ); }
   int16_t getAccY() { return getReg( mpu6050_reg_a_yh ); }
   int16_t getAccZ() { return getReg( mpu6050_reg_a_zh ); }
   int     getAccAll( int16_t *acc ){ return getRegs( mpu6050_reg_a_xh, 3, acc ); }
   int16_t getGyroX() { return getReg( mpu6050_reg_g_xh ); }
   int16_t getGyroY() { return getReg( mpu6050_reg_g_yh ); }
   int16_t getGyroZ() { return getReg( mpu6050_reg_g_zh ); }
   int     getGyroAll( int16_t *gyro ){ return getRegs( mpu6050_reg_g_xh, 3, gyro ); }
   int16_t getTemp() { return fixTemp( getReg( mpu6050_reg_temph )); }
   void    getAll( int16_t *all_data ){
     getRegs( mpu6050_reg_a_xh, 8, all_data );
     all_data[3] = (int16_t)( fixTemp( all_data[3] ));
   }
  private:
};

#endif
