#include <oxc_ds3231.h>

#define I2C_TO 100

void bcd_to_char2( uint8_t bcd, char *s )
{
  if( !s ) { return; };
  s[1] = '0' + ( bcd & 0x0F );
  s[0] = '0' + ( bcd >> 4 );
}

int DS3231::sendByteReg( uint16_t reg, uint8_t d )
{
  return HAL_I2C_Mem_Write( &i2ch, addr2, reg, I2C_MEMADD_SIZE_8BIT, &d, 1, I2C_TO );
}

int DS3231::setCtl( uint8_t ctl )
{
  return sendByteReg( reg_ctl, ctl );
}

uint8_t DS3231::getStatus()
{
  uint8_t v = 0;
  HAL_I2C_Mem_Read( &i2ch, addr2, reg_status, I2C_MEMADD_SIZE_8BIT, &v, 1, I2C_TO );
  return v;
}

int DS3231::setTime( uint8_t  hour,  uint8_t min, uint8_t  sec )
{
  if( sec >  59 ) { sec = 0; }
  if( min >  59 ) { min = 0; }
  if( hour > 24 ) { hour = 0; }
  uint8_t buf[4];
  buf[0] = uint8_to_bcd( sec );
  buf[1] = uint8_to_bcd( min );
  buf[2] = uint8_to_bcd( hour );
  return HAL_I2C_Mem_Write( &i2ch, addr2, reg_sec, I2C_MEMADD_SIZE_8BIT, buf, 3, I2C_TO );
}

int DS3231::getTime( uint8_t *hour, uint8_t *min, uint8_t *sec )
{
  uint8_t buf[4];
  auto rc = HAL_I2C_Mem_Read( &i2ch, addr2, reg_sec, I2C_MEMADD_SIZE_8BIT, buf, 3, I2C_TO );
  if( rc != HAL_OK ) {
    return rc;
  }
  buf[2] &= 0x3F; // hide 13/24 switch
  uint8_t h = bcd_to_uint8( buf[2] ), m = bcd_to_uint8( buf[1] ), s = bcd_to_uint8( buf[0] );
  if( hour ) { *hour = h; };
  if( min  ) { *min  = m; };
  if( sec  ) { *sec  = s; };
  return rc;
}

int DS3231::getTimeStr( char *s )
{
  uint8_t buf[4];
  if( !s ) { return HAL_ERROR; }
  auto rc = HAL_I2C_Mem_Read( &i2ch, addr2, reg_sec, I2C_MEMADD_SIZE_8BIT, buf, 3, I2C_TO );
  if( rc != HAL_OK ) {
    return rc;
  }
  bcd_to_char2( buf[2], s   ); s[2] = ':';  // HH:
  bcd_to_char2( buf[1], s+3 ); s[5] = ':';  // HH:MM:
  bcd_to_char2( buf[0], s+6 ); s[8] = '\0'; // HH:MM:SS
  return rc;
}

int DS3231::setDate( int16_t  year,  uint8_t month, uint8_t  day )
{
  year -= year_base;
  if( year < 0 || year > 198 ) { year  = 0; }
  if( month > 12 || month < 1 ) { month = 1; }; // 1 = Jan
  if( year > 99 ) { month |= 0x80; }; // century bit
  if( day < 1 || day > 31 ) { day = 1; }
  uint8_t buf[4];
  buf[0] = uint8_to_bcd( day );
  buf[1] = uint8_to_bcd( month );
  buf[2] = uint8_to_bcd( (uint8_t)year );
  return HAL_I2C_Mem_Write( &i2ch, addr2, reg_day, I2C_MEMADD_SIZE_8BIT, buf, 3, I2C_TO );
}

int DS3231::getDate( int16_t *year, uint8_t *month, uint8_t *day )
{
  uint8_t buf[4];
  auto rc = HAL_I2C_Mem_Read( &i2ch, addr2, reg_day, I2C_MEMADD_SIZE_8BIT, buf, 3, I2C_TO );
  if( rc != HAL_OK ) {
    return rc;
  }
  int16_t y = year_base;
  if( buf[1] & 0x80 ) { y += 100; }
  buf[1] &= 0x1F;

  uint8_t ye = bcd_to_uint8( buf[2] ), mo = bcd_to_uint8( buf[1] ), da = bcd_to_uint8( buf[0] );
  y += ye;

  if( year )   { *year = ye; };
  if( month  ) { *month  = mo; };
  if( day  )   { *day  = da; };
  return rc;
}

int DS3231::getDateStr( char *s )
{
  uint8_t buf[4];
  if( !s ) { return HAL_ERROR; }
  auto rc = HAL_I2C_Mem_Read( &i2ch, addr2, reg_day, I2C_MEMADD_SIZE_8BIT, buf, 3, I2C_TO );
  if( rc != HAL_OK ) {
    return rc;
  }
  s[0] = '2';
  s[1] = ( buf[1] & 0x80 ) ? '1' : '0';
  buf[1] &= 0x1F;
  bcd_to_char2( buf[2], s+2 ); s[4] = ':';   // YYYY:
  bcd_to_char2( buf[1], s+5 ); s[7] = ':';   // YYYY:MM:
  bcd_to_char2( buf[0], s+8 ); s[10] = '\0'; // YYYY:MM:DD
  return rc;
}



