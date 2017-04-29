#ifndef _OXC_HD44780_I2C_H
#define _OXC_HD44780_I2C_H

#include <oxc_i2c.h>

class HD44780_i2c {
  public:
    enum {
      lcd_8b_mode   = 0x30,
      lcd_4b_mode   = 0x20,
      lcd_mode_2row = 0x08,
      lcd_cmd_cls   = 0x01,
      lcd_cmd_home  = 0x02,
      lcd_cmd_onoff = 0x08,
      lcd_cmd_bit_on= 0x04,
      lcd_cmd_cur_on= 0x02,
      lcd_cmd_blk_on= 0x01,
      lcd_bit_rs    = 0x01,
      lcd_bit_rw    = 0x02, // unused, set to 0 always
      lcd_bit_e     = 0x04,
      lcd_bit_led   = 0x08,
      lcd_def_addr  = 0x27
    };
    HD44780_i2c( DevI2C &a_dev, uint8_t d_addr = lcd_def_addr )
     : dev( a_dev ), addr( d_addr ) {};
    void setAddr( uint8_t d_addr ) { addr = d_addr; };
    uint8_t getAddr() const { return addr; }
    int  getState() const { return dev.getState(); };
    void init_4b( bool is_2row = true );
    void wr4( uint8_t v, bool is_data );
    void strobe( uint8_t v );
    void putch( char c ) { wr4( c, true ); }
    void putxych( uint8_t x, uint8_t y, char c ) { gotoxy( x, y ); putch( c ); }
    void cmd( uint8_t cmd ) { wr4( cmd, false ); }
    void puts( const char *s );
    void gotoxy( uint8_t x, uint8_t y );
    void cls()   { cmd( lcd_cmd_cls );  delay_ms( 4 ); }
    void home()  { cmd( lcd_cmd_home ); delay_ms( 2 ); }
    void on()       { mod |=  lcd_cmd_bit_on; cmd( mod | lcd_cmd_onoff ); }
    void off()      { mod &= ~lcd_cmd_bit_on; cmd( mod | lcd_cmd_onoff ); }
    void curs_on()  { mod |=  lcd_cmd_cur_on; cmd( mod | lcd_cmd_onoff ); }
    void curs_off() { mod &= ~lcd_cmd_cur_on; cmd( mod | lcd_cmd_onoff ); }
    void led_on()   { led_state = lcd_bit_led;cmd( mod | lcd_cmd_onoff ); }
    void led_off()  { led_state = 0;          cmd( mod | lcd_cmd_onoff ); }

  protected:
   DevI2C &dev;
   uint8_t addr;
   uint8_t mod = 0;
   uint8_t led_state = lcd_bit_led;
   static const constexpr uint8_t n_lines { 4 };
   static const uint8_t line_addr[n_lines];
   const uint8_t cpl = 40; // 40 char/line
};


#endif
