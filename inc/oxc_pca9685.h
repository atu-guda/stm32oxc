#ifndef _OXC_PCA9685_H
#define _OXC_PCA9685_H

#include <algorithm>
#include <oxc_i2c.h>

// PCA9685: 16-channel 12-bit PWM controller, good for servo ...

class PCA9685 : public I2CClient {
  public:
   enum {
     addr_def      = 0x40,
     reg_mode0     = 0x00,
     reg_mode1     = 0x01,
     reg_led0s     = 0x06, // 6 = on_low, 7 = on_high, 8 = off_low, 9 = off_high+
     reg_all_s     = 0xFA, // FA-FD
     reg_prescale  = 0xFE, // 121 = 50Hz, 30 - 200 Hz
     bit_onoff     = 0x10, // on and off bits on *_high
     mask_high     = 0x0F, // value bits in *_high
     cmd_on_inc    = 0x20, // to mode0
     cmd_off       = 0x10,
     base_freq     = 25000000, // default inner frequency
     base_freq_mhz = 25,       // default inner frequency in MHz
     presc_24Hz    = 253,
     presc_50Hz    = 121, // prescale for 50Hz
     presc_100Hz   = 60,
     presc_200Hz   = 30,
     presc_def     = presc_50Hz,
     servo_us_min  = 500,
     servo_us_max  = 2500,
     servo_in_max  = 1000,
     v_out_mask    = 0x0FFF, // 12bit : most used 0x01FF
     n_ch          = 16
   };
   PCA9685( DevI2C &d_i2c, uint8_t d_addr = addr_def, uint8_t a_presc = presc_def )
     : I2CClient( d_i2c, d_addr ), presc(a_presc)  {}
   bool init( uint8_t presc_new = 0 ); // 0 means use setted before
   void setPresc( uint8_t presc_new ) { init( presc_new ); }
   void setFreq( uint32_t freq ) { init( freq2prec( freq ) ); } // min is 24 Hz, max is 1524 Hz
   uint8_t getPresc() const { return presc; }
   uint32_t getFreq() const { return base_freq / ( 4096 * ( presc + 1 ) ); }
   uint32_t getPeriod_us() const { return 4096 * ( presc + 1 ) / base_freq_mhz; }
   uint16_t us2v( uint32_t us ) const { return ( 2 * ( us + 1 )* base_freq_mhz / ( presc + 1 ) ) / 2 ; }
   void sleep();
   void set( uint8_t ch, uint16_t on, uint16_t off ); // 0-4096
   void setServoMinMax( uint32_t min_us, uint32_t max_us );
   uint16_t servo2v( int a ) const; // converts value (-1000:1000) to 'off' ticks
   uint32_t servo2t( int a ) const; // converts value (-1000:1000) to time in us
   void setServo( uint8_t ch, int val ); // value is signed!
   void off( uint8_t ch ); // to on - use set!
   void setAllServo( int val );
   void offAll();

   static inline constexpr uint8_t freq2prec( uint32_t freq ) // conversion frequency to prescaler value with limits
     { return (uint8_t) std::clamp( ( base_freq / ( 4096 * freq ) ) - 1, 3ul, 255ul ); }
                                              //
   static inline constexpr uint8_t ch2regOff( uint8_t ch_idx ) { return reg_led0s + 4*ch_idx + 2; } // 2: reg(off)
   static inline constexpr uint8_t ch2regOn(  uint8_t ch_idx ) { return reg_led0s + 4*ch_idx; } // 2: reg(on)
  protected:
   uint8_t presc;
   uint32_t servo_min = servo_us_min;
   uint32_t servo_max = servo_us_max;
   uint32_t servo_mid = ( servo_us_max + servo_us_min ) / 2 ;
   uint32_t servo_dvt = ( servo_us_max - servo_us_min );
};

#endif

