#ifndef _OXC_DS3231_H
#define _OXC_DS3231_H

#include <oxc_base.h>

// inner regs: 1-byte addr

inline uint8_t uint8_to_bcd( uint8_t v ) {
  return ( v % 10 ) + (( v / 10 ) << 4);
}

inline uint8_t bcd_to_uint8( uint8_t bcd ) {
  return ( bcd & 0x0F ) + 10 * ( bcd >> 4 );
}

void bcd_to_char2( uint8_t bcd, char *s );

class DS3231 {
  public:
   enum {
     def_addr     = 0x68,
     alldata_sz   = 0x12,
     reg_sec      = 0x00, // BCD
     reg_min      = 0x01,
     reg_hour     = 0x02, // +b5:PM/20h, b6: 12hour
     reg_wday     = 0x03, // 1-7
     reg_day      = 0x04, // 1-31
     reg_month_s  = 0x05, // mouth + b7:Century
     reg_year     = 0x06,
     reg_ctl      = 0x0E,
     reg_status   = 0x0F,
     reg_aging    = 0x10,
     reg_temp2    = 0x11,
     reg_temp1    = 0x12,
     ctl_no_osc   = 0x80, // disable oscillator on Vbat
     ctl_bbsw_en  = 0x40, // enable square wave on battery
     ctl_temp_cnv = 0x20, // force temperature convert
     ctl_rate_1Hz = 0x00, // sqate rate freq
     ctl_rate_1024Hz = 0x08,
     ctl_rate_4096Hz = 0x10,
     ctl_rate_8192Hz = 0x18,
     ctl_irqcn       = 0x04, // output is irq, not wave
     ctl_a2ie        = 0x02, // enable alarm 2 irq
     ctl_a1ie        = 0x02, // enable alarm 1 irq
     year_base       = 2000, // zero year
   };

   DS3231( I2C_HandleTypeDef &d_i2ch, uint8_t d_addr = def_addr )
     : i2ch( d_i2ch ), addr2( d_addr<<1 ) {};
   int sendByteReg( uint16_t reg, uint8_t d );
   int setCtl( uint8_t ctl );
   uint8_t getStatus();
   int setTime( uint8_t  hour,  uint8_t min, uint8_t  sec );
   int getTime( uint8_t *hour, uint8_t *min, uint8_t *sec );
   int getTimeStr( char *s ); // HH:MM:SS = 9 bytes minimum
   int setDate( int16_t  year,  uint8_t month, uint8_t  day );
   int getDate( int16_t *year, uint8_t *month, uint8_t *day );
   int getDateStr( char *s ); // YYYY:MM:DD = 11 bytes minimum
  private:
   I2C_HandleTypeDef &i2ch;
   uint8_t addr2;
};

#endif
