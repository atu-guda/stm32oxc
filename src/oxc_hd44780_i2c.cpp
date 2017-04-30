#include <oxc_hd44780_i2c.h>

const uint8_t HD44780_i2c::line_addr[n_lines]  { 0x00, 0x40, 0x14, 0x54 };

void HD44780_i2c::init_4b( bool is_2row )
{
  delay_ms( 40 );
  strobe( lcd_8b_mode );  delay_ms( 4 );
  strobe( lcd_8b_mode );  delay_ms( 4 );
  strobe( lcd_8b_mode );  delay_ms( 4 );
  strobe( lcd_4b_mode | ( is_2row ? lcd_mode_2row : 0 ) );  delay_ms( 4 );
  cls();
  on();
}


void HD44780_i2c::wr4( uint8_t v, bool is_data )
{
  uint8_t b = ( is_data ) ? lcd_bit_rs : 0;
  b |= led_state;
  uint8_t v1 = ( v & 0xF0 );
  uint8_t d = v1 | lcd_bit_e | b;
  send( d );
  delay_mcs( 1 );
  d = v1 | led_state | b;
  send( d );
  delay_mcs( 1 );
  v1 = v << 4;
  d = v1 | lcd_bit_e | b;
  send( d );
  delay_mcs( 1 );
  d = v1 | b;
  send( d );
}

void HD44780_i2c::strobe( uint8_t v )
{
  uint8_t d = v | lcd_bit_e | led_state;
  send( d );
  delay_mcs( 2 );
  d = v | led_state;
  send( d );
}

void HD44780_i2c::puts( const char *s )
{
  if( !s ) {
    return;
  }
  for( ; *s; ++s ) {
    putch( *s );
  };
}

void HD44780_i2c::gotoxy( uint8_t x, uint8_t y )
{
  if( y > n_lines ) {
    return;
  }
  uint8_t ofs = x + line_addr[y];
  ofs |= 0x80; // cmd
  cmd( ofs );
}

