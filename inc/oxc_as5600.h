#ifndef _OXC_AS5600_H
#define _OXC_AS5600_H

// AS5600 magnetic angle sensor 12 bit I2C

#include <oxc_i2c.h>

// inner regs: 1-byte addr

class AS5600 : public I2CClient {
  public:
   enum RegNums {
     def_addr           = 0x36,
     reg_zmco           = 0x00,
     reg_zpos_high      = 0x01,
     reg_zpos_low       = 0x02,
     reg_mpos_high      = 0x03,
     reg_mpos_low       = 0x04,
     reg_mang_high      = 0x05,
     reg_mang_low       = 0x06,
     reg_conf_high      = 0x07,
     reg_conf_low       = 0x08,
     reg_raw_angle_high = 0x0C,
     reg_raw_angle_low  = 0x0D,
     reg_angle_high     = 0x0E,
     reg_angle_low      = 0x0F,
     reg_status         = 0x0B,
     reg_agc            = 0x1A,
     reg_magnitude_high = 0x1B,
     reg_magnitude_low  = 0x1C,
     reg_burn           = 0xFF
   };
   enum CfgBits {
     cfg_pwr_mode_nom    = 0x00,
     cfg_pwr_mode_lpm1   = 0x01,
     cfg_pwr_mode_lpm2   = 0x02,
     cfg_pwr_mode_lpm3   = 0x03,
     cfg_pwr_mode_dfl    = cfg_pwr_mode_nom,
     cfg_hyst_off        = 0x00,
     cfg_hyst_1lsb       = 0x01 << 2,
     cfg_hyst_2lsb       = 0x02 << 2,
     cfg_hyst_3lsb       = 0x03 << 2,
     cfg_hyst_dfl        = cfg_hyst_off,
     cfg_out_full        = 0x00,
     cfg_out_reduced     = 0x01 << 4,
     cfg_out_pwm         = 0x02 << 4,
     cfg_out_dfl         = cfg_out_full,
     cfg_pwm_freq_115hz  = 0x00,
     cfg_pwm_freq_230hz  = 0x01 << 6,
     cfg_pwm_freq_460hz  = 0x02 << 6,
     cfg_pwm_freq_920hz  = 0x03 << 6,
     cfg_pwm_freq_dfl    = cfg_pwm_freq_115hz,
     cfg_sfilt_16x       = 0x00,
     cfg_sfilt_8x        = 0x01 << 8,
     cfg_sfilt_4x        = 0x02 << 8,
     cfg_sfilt_2x        = 0x03 << 8,
     cfg_sfilt_dfl       = cfg_sfilt_16x,
     cfg_ffilt_slow_only = 0x00 << 10,
     cfg_ffilt_6lsb      = 0x01 << 10,
     cfg_ffilt_7lsb      = 0x02 << 10,
     cfg_ffilt_9lsb      = 0x03 << 10,
     cfg_ffilt_18lsb     = 0x04 << 10,
     cfg_ffilt_21lsb     = 0x05 << 10,
     cfg_ffilt_24lsb     = 0x06 << 10,
     cfg_ffilt_10lsb     = 0x07 << 10,
     cfg_ffilt_dfl       = cfg_ffilt_slow_only,
     cfg_watchdog_off    = 0x00,
     cfg_watchdog_on     = 0x01 << 13,
     cfg_watchdog_dfl    = cfg_watchdog_on
   };
   enum StatusBits {
     status_magn_high    =  0x08,
     status_magn_low     =  0x10,
     status_magn_detect  =  0x20
   };

   AS5600( DevI2C &a_dev, uint8_t d_addr = def_addr )
     : I2CClient( a_dev, d_addr ) {};
   static constexpr int32_t to_mDeg( int32_t ang12bit ) { return ( ang12bit * 360000 / 4096 ); };
   static constexpr int32_t from_mDeg( int32_t mDeg ) { return ( mDeg * 4096 / 360000 ); };
   uint16_t getReg( uint8_t reg ){ return recv_reg1_16bit_rev( reg ); }; // reg is 16-bit
   bool    getRegs( uint8_t reg1, uint8_t n, uint16_t *data ){ return recv_reg1_16bit_n_rev( reg1, (uint16_t*)(data), n ) == n ; }

   uint16_t getAngleRaw() { return getReg( reg_raw_angle_high ); }
   uint16_t getAngle()    { return getReg( reg_angle_high ); };
   uint32_t getAngle_mDeg() { return to_mDeg( getAngle() ); }
   uint8_t  getStatus()   { return recv_reg1_8bit( reg_status ); };

   uint16_t getCfg()      { return getReg( reg_conf_high ); };
   bool setCfg( uint16_t cfg ) { return send_reg1_16bit_rev ( reg_conf_high, cfg ) == 2 ; };

   bool setStartPos( uint16_t pos ) { return send_reg1_16bit_rev ( reg_zpos_high, pos ) == 2 ; };
   bool setStartPosCurr() { return send_reg1_16bit_rev ( reg_zpos_high, getAngleRaw() ) == 2 ; };
   bool setStopPos( uint16_t pos )  { return send_reg1_16bit_rev ( reg_mpos_high, pos ) == 2 ; };
   bool setMaxAngle( uint16_t angle )  { return send_reg1_16bit_rev ( reg_mang_high, angle ) == 2 ; };
   bool setPositiveRotationDirection( uint8_t dir );
   bool isMagnetDetected() { return bool( getStatus() & status_magn_detect ); }
   uint8_t getAGCSetting() { return recv_reg1_8bit( reg_agc ); }
   uint16_t getCORDICMagnitude() { return getReg( reg_magnitude_high ); }
  protected:
   int32_t alg_ext {0};
   //
};



// #define AS5600_AGC_MIN_GAIN_OVERFLOW (uint8_t)(1UL << 3) #<{(|Error bit indicates b-field is too string |)}>#
// #define AS5600_AGC_MAX_GAIN_OVERFLOW (uint8_t)(1UL << 4) #<{(|Error bit indicates b-field is too weak |)}>#
// #define AS5600_MAGNET_DETECTED (uint8_t)(1UL << 5)       #<{(|Status bit indicates b-field is detected |)}>#
//
// #define AS5600_DIR_CW 1
// #define AS5600_DIR_CCW 2

#endif

