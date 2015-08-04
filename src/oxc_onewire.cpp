#include <oxc_onewire.h>

void OneWire::write_buf( const uint8_t *b, uint16_t l )
{
  if( !b || !l ) { return; }
  for( uint16_t i=0; i<l; ++i,++b ) {
    uint8_t c = *b;
    for( uint8_t j=0; j<8; ++j ) {
      write1bit( c & 1 );
      c >>= 1;
    }
    delay_ms( 1 );
    // d_mcs( T_R_L );
  };
}

void OneWire::read_buf( uint8_t *b, uint16_t l )
{
  if( !b || !l ) { return; }
  for( uint16_t i=0; i<l; ++i,++b ) {
    uint8_t c = 0;
    for( uint8_t j=0; j<8; ++j ) {
      if( read1bit() ) { c |= (1<<j); }
    }
    *b = c;
    delay_ms( 1 );
    // d_mcs( T_R_L );
  };
}

bool OneWire::gcmd( const uint8_t *addr, uint8_t cmd,
       const uint8_t *snd, uint8_t s_sz, uint8_t *rcv, uint16_t r_sz  )
{
  if( !reset() ) {
    return false;
  }

  if( addr ) {
    write1byte( CMD_MATCH_ROM );
    write_buf( addr, 8 );
  } else {
    write1byte( CMD_SKIP_ROM );
  }

  write1byte( cmd );

  if( snd && s_sz ) {
    write_buf( snd, s_sz );
  }

  if( rcv && r_sz ) {
    read_buf( rcv, r_sz );
  } else {
    return true;
  };

  if( !check_crc ) {
    return true;
  }

  uint8_t cr = calcCrc( rcv, r_sz );

  return (cr == rcv[r_sz-1] );
}

bool OneWire::searchRom( const uint8_t *snd, uint8_t *rcv, uint16_t r_sz  )
{
  if( !snd || !rcv || !reset() ) {
    return false;
  }
  write1byte( CMD_SEARCH_ROM );
  write_buf( snd, 8 );
  read_buf( rcv, 8 );
  if( !check_crc ) {
    return true;
  }
  uint8_t cr = calcCrc( rcv, r_sz );
  return cr == rcv[r_sz-1];
}

bool OneWire::readRom( uint8_t *rcv, uint16_t r_sz  )
{
  if( !rcv || !reset() ) {
    return false;
  }
  write1byte( CMD_READ_ROM );
  read_buf( rcv, r_sz );
  if( !check_crc ) {
    return true;
  }
  uint8_t cr = calcCrc( rcv, r_sz );
  return cr == rcv[r_sz-1];
}

bool OneWire::matchRom( const uint8_t *addr, uint8_t cmd, uint8_t *rcv, uint16_t r_sz  )
{
  return gcmd( addr, cmd, nullptr, 0, rcv, r_sz );
}

bool OneWire::skipRom( uint8_t cmd, uint8_t *rcv, uint16_t r_sz  )
{
  return gcmd( nullptr, cmd, nullptr, 0, rcv, r_sz );
}

uint8_t OneWire::calcCrc( const uint8_t *b, uint16_t l )
{
  uint8_t crc = 0;

  for( uint16_t j=0; j<l-1; ++j ) {
    uint8_t ib = b[j];
    for( uint8_t i = 0; i<8; ++i ) {
      uint8_t mx = ( crc ^ ib ) & 0x01;
      crc >>= 1;
      if( mx ) {
        crc ^= 0x8C;
      }
      ib >>= 1;
    }
  }

  return crc;
}

