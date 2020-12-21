#include <oxc_ds3231.h>


uint8_t DS3231::getStatus()
{
  uint8_t v = 0;
  recv_reg1( reg_status, &v, 1 );
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
  return send_reg1( reg_sec, buf, 3 );
}

int DS3231::getTime( uint8_t *hour, uint8_t *min, uint8_t *sec )
{
  uint8_t buf[4];
  auto rc = recv_reg1( reg_sec, buf, 3 );
  if( rc < 1  ) {
    return 0;
  }
  buf[2] &= 0x3F; // hide 13/24 switch
  uint8_t h = bcd_to_uint8( buf[2] ), m = bcd_to_uint8( buf[1] ), s = bcd_to_uint8( buf[0] );
  if( hour ) { *hour = h; };
  if( min  ) { *min  = m; };
  if( sec  ) { *sec  = s; };
  return 1;
}

int DS3231::getTimeStr( char *s )
{
  uint8_t buf[4];
  if( !s ) { return 0; }
  s[0] = '\0';
  auto rc = recv_reg1( reg_sec, buf, 3 );
  if( rc < 1 ) {
    return 0;
  }
  bcd_to_char2( buf[2], s   ); s[2] = ':';  // HH:
  bcd_to_char2( buf[1], s+3 ); s[5] = ':';  // HH:MM:
  bcd_to_char2( buf[0], s+6 ); s[8] = '\0'; // HH:MM:SS
  return 1;
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
  return send_reg1( reg_day, buf, 3 );
}

int DS3231::getDate( int16_t *year, uint8_t *month, uint8_t *day )
{
  uint8_t buf[4];
  auto rc = recv_reg1( reg_day, buf, 3 );
  if( rc < 0 ) {
    return 0;
  }
  int16_t y = year_base;
  if( buf[1] & 0x80 ) { y += 100; }
  buf[1] &= 0x1F;

  uint8_t ye = bcd_to_uint8( buf[2] ), mo = bcd_to_uint8( buf[1] ), da = bcd_to_uint8( buf[0] );
  y += ye;

  if( year )   { *year = y; };
  if( month  ) { *month  = mo; };
  if( day  )   { *day  = da; };
  return 1;
}

int DS3231::getDateStr( char *s )
{
  uint8_t buf[4];
  if( !s ) { return 0; }
  s[0] = '\0';
  auto rc = recv_reg1( reg_day, buf, 3 );
  if( rc < 1 ) {
    return 0;
  }
  s[0] = '2';
  s[1] = ( buf[1] & 0x80 ) ? '1' : '0';
  buf[1] &= 0x1F;
  bcd_to_char2( buf[2], s+2 ); s[4] = ':';   // YYYY:
  bcd_to_char2( buf[1], s+5 ); s[7] = ':';   // YYYY:MM:
  bcd_to_char2( buf[0], s+8 ); s[10] = '\0'; // YYYY:MM:DD
  return rc;
}

int DS3231::getDateTimeStr( char *s )
{
  uint8_t buf[4];
  if( !s ) { return 0; }
  s[0] = '\0';

  auto rc = recv_reg1( reg_day, buf, 3 );
  if( rc < 1 ) {
    return 0;
  }
  s[0] = '2';
  s[1] = ( buf[1] & 0x80 ) ? '1' : '0';
  buf[1] &= 0x1F;
  bcd_to_char2( buf[2], s+2 );    // YYYY
  bcd_to_char2( buf[1], s+4 );    // YYYYMM
  bcd_to_char2( buf[0], s+6 ); s[8] = '_';  s[9] = '\0'; // YYYYMMDD_

  rc = recv_reg1( reg_sec, buf, 3 );
  if( rc < 1 ) {
    return 0;
  }
  bcd_to_char2( buf[2], s+9  );  // YYYYMMDD_HH
  bcd_to_char2( buf[1], s+11 );  // YYYYMMDD_HHMM
  bcd_to_char2( buf[0], s+13  ); s[15] = '\0'; // YYYYMMDD_HHMMSS

  return rc;
}


int DS3231::getDateTime( int *t )
{
  if( !t ) {
    return 0;
  }
  for( int i=0; i<6; ++i ) {
    t[i] = 0;
  }

  uint8_t hour, min, sec;
  if( ! getTime( &hour, &min, &sec ) ) {
    return 0;
  }

  int16_t year;
  uint8_t month, day;
  if( ! getDate( &year, &month, &day ) ) {
    return 0;
  }
  t[0] = year; t[1] = month; t[2] = day;
  t[3] = hour; t[4] = min;   t[5] = sec;

  return 1;
}


