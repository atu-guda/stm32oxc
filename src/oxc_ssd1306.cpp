#include <oxc_ssd1306.h>

using namespace std;


int  SSD1306::cmd1( uint8_t cmd )
{
  uint8_t xcmd[] = { CMD_1, cmd };
  return dev.send( xcmd, 2, addr );
}

int  SSD1306::cmd2( uint8_t cmd, uint8_t val )
{
  uint8_t xcmd[] = { CMD_N, cmd, val };
  return dev.send( xcmd, 3, addr );
}

int  SSD1306::data1( uint8_t d )
{
  uint8_t xcmd[] = { DATA_1, d };
  return dev.send( xcmd, 2, addr );
}


int  SSD1306::init()
{
  // static const constexpr uint8_t on_cmd[] = { 0x00, 0x8D, 0x14, 0xAF };
  uint8_t on_cmd[] = { 0x00, 0x8D, 0x14, 0xAF };
  return dev.send( on_cmd, 4, addr );
}



int  SSD1306::out( PixBuf1V &pb )
{
  uint8_t go_00[] = { 0x00, 0xB0, 0x00, 0x10 };
  dev.send( go_00, 4, addr );
  uint8_t *buf = pb.fb();
  --buf; // one byte for cmd
  *buf = DATA_N;
  return dev.send( buf, MEM_SZ+1, addr );
}

