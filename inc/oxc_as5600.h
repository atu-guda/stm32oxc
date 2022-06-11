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
     cfg_power_mode_nom        = 0x01,
     cfg_power_mode_lpm1       = 0x02,
     cfg_power_mode_lpm2       = 0x03,
     cfg_power_mode_lpm3       = 0x04,
     cfg_power_mode_default    = cfg_power_mode_nom,
     cfg_hysteresis_off        = 1,
     cfg_hysteresis_1lsb       = 2,
     cfg_hysteresis_2lsb       = 3,
     cfg_hysteresis_3lsb       = 4,
     cfg_hysteresis_default    = cfg_hysteresis_off,
     cfg_output_full           = 1,
     cfg_output_reduced        = 2,
     cfg_output_pwm            = 3,
     cfg_output_default        = cfg_output_full,
     cfg_pwm_freq_115hz        = 1,
     cfg_pwm_freq_230hz        = 2,
     cfg_pwm_freq_460hz        = 3,
     cfg_pwm_freq_920hz        = 4,
     cfg_pwm_freq_default      = cfg_pwm_freq_115hz,
     cfg_slow_filter_16x       = 1,
     cfg_slow_filter_8x        = 2,
     cfg_slow_filter_4x        = 3,
     cfg_slow_filter_2x        = 4,
     cfg_slow_filter_default   = cfg_slow_filter_16x,
     cfg_fast_filter_slow_only = 1,
     cfg_fast_filter_6lsb      = 2,
     cfg_fast_filter_7lsb      = 3,
     cfg_fast_filter_9lsb      = 4,
     cfg_fast_filter_18lsb     = 5,
     cfg_fast_filter_21lsb     = 6,
     cfg_fast_filter_24lsb     = 7,
     cfg_fast_filter_10lsb     = 8,
     cfg_fast_filter_default   = cfg_fast_filter_slow_only,
     cfg_watchdog_off          = 1,
     cfg_watchdog_on           = 2,
     cfg_watchdog_default      = cfg_watchdog_on
   };
   enum StatusBits {
     status_scale_0_88     =  0x00,
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
   uint16_t getStatus()   { return recv_reg1_8bit( reg_status ); };
   uint16_t getCfg();
   bool setCfg( uint16_t cfg );

   bool setStartPos( uint16_t pos );
   bool setStopPos( uint16_t pos );
   bool setMaxAngle( uint16_t angle );
   bool setPositiveRotationDirection( uint8_t dir );
   bool setLowPowerMode( uint8_t mode );
   bool setHysteresis( uint8_t hysteresis );
   bool setOutputMode( uint8_t mode, uint8_t freq );
   bool setSlowFilter( uint8_t mode );
   bool setFastFilterThreshold( uint8_t threshold );
   bool setWatchdogTimer( uint8_t mode );
   bool isMagnetDetected();
   bool getAGCSetting( uint8_t *const agc);
   bool getCORDICMagnitude( uint16_t *const mag );
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

