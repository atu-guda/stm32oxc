#include <cstring>
// for std::size
#include <iterator>

#include <oxc_ssd1306.h>


int  SSD1306::cmd1( uint8_t cmd )
{
  uint8_t xcmd[] = { CMD_1, cmd };
  return send( xcmd, 2 );
}

int  SSD1306::cmd2( uint8_t cmd, uint8_t val )
{
  uint8_t xcmd[] = { CMD_N, cmd, val };
  return send( xcmd, 3 );
}

int SSD1306::cmdN( const uint8_t *cmds, unsigned n )
{
  uint8_t xcmd[n+1];
  xcmd[0] = ( n >  1 ) ? SSD1306::CMD_N: SSD1306::CMD_1;
  std::memcpy( xcmd+1, cmds, n );
  return send( xcmd, n+1 );
}

int  SSD1306::data1( uint8_t d )
{
  uint8_t xcmd[] = { DATA_1, d };
  return send( xcmd, 2 );
}


int  SSD1306::init( bool isY32 /* = false */ )
{
  //  { 0x00, 0x8D, 0x14, 0xAF }; // BUG: why not?
  static const constexpr uint8_t on_cmd[] =
  {
    CMD_N,        // 0x00
    0x8D, 0x14,   // charge pump on
    0xA0,         // noflip
    0xA4,         // RAM
    0xA6,         // noinverse
    0x20, 0x00,  // ???
    0x22, 0x00, 0x3F, // good start
    0xAF // ON
  };
  send( on_cmd, std::size(on_cmd) );

  static const constexpr uint8_t on_cmd1_64[] = { 0x00, 0xDA, 0x12 };
  static const constexpr uint8_t on_cmd1_32[] = { 0x00, 0xDA, 0x02 };
  if( isY32 ) {
    y_sz = 32;
    send( on_cmd1_32, std::size(on_cmd1_32) );
  } else {
    y_sz = 64;
    send( on_cmd1_64, std::size(on_cmd1_64) );
  }
  return 1;
}



int  SSD1306::out( PixBuf1V &pb )
{
  uint8_t go_00[] = { 0x00, 0xB0, 0x00, 0x10 };
  send( go_00, 4 );
  uint8_t *buf = pb.fb();
  --buf; // one byte for cmd
  *buf = DATA_N;
  return send( buf, getMemSize()+1 );
}

