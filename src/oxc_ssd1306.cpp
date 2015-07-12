#include <oxc_ssd1306.h>

using namespace std;


void SSD1306::cmd1( uint8_t cmd )
{
  uint8_t xcmd[] = { CMD_1, cmd };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  xcmd, 2, I2C_TO );
}

void SSD1306::cmd2( uint8_t cmd, uint8_t val )
{
  uint8_t xcmd[] = { CMD_N, cmd, val };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  xcmd, 3, I2C_TO );
}

void SSD1306::data1( uint8_t d )
{
  uint8_t xcmd[] = { DATA_1, d };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  xcmd, 2, I2C_TO );
}


void SSD1306::init()
{
  uint8_t on_cmd[] = { 0x00, 0x8D, 0x14, 0xAF };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  on_cmd, 4, I2C_TO );
}



void SSD1306::out( PixBuf1V &pb )
{
  uint8_t go_00[] = { 0x00, 0xB0, 0x00, 0x10 };
  HAL_I2C_Master_Transmit( &i2ch, addr2,  go_00, 4, I2C_TO );
  uint8_t *buf = pb.fb();
  --buf; // one byte for cmd
  *buf = DATA_N;
  HAL_I2C_Master_Transmit( &i2ch, addr2,  buf, MEM_SZ+1, I2C_TO );
}
