#include <oxc_spi_ads1220.h>

// debug
#include <cstring>


void ADS1220::init()
{
  // spi_d.initSPI();
  reset();
  delay_ms( 100 );
  for( uint8_t i=0; i < n_cfgs; ++i ) {
    writeReg( i, cfgs[i] );
  }
  delay_ms( 100 );
  // TODO: re-read and compare?
  // for( auto [i,r] : cfgs | ranges::views::enumerate ) {}
}

void ADS1220::read_config()
{
  for( uint8_t i=0; i < n_cfgs; ++i ) {
    cfgs[i] = readReg( i );
  }
}

std::optional<int32_t> ADS1220::read_nowait()
{
  uint8_t b[3];
  int rc = spi_d.recv( b, 3 );
  if( rc != 3 ) {
    return {};
  }


  int32_t b24 = b[0]; // TODO: bit_cast
  b24 = (b24 << 8) | b[1];
  b24 = (b24 << 8) | b[2];

  b24 = (b24 << 8);
  int32_t r = (b24 >> 8);
  return r;
}

std::optional<int32_t> ADS1220::read_wait()
{
  bool good = false;
  if( ndrdy ) {
    for( int i=0; i<max_wait; ++i ) {
      if( ndrdy->read() == 0 ) {
        good = true;
        break;
      }
      delay_ms( 2 );
    }
  } else {
    delay_ms( max_wait );
    good = true;
  }
  if( good ) {
    return read_nowait();
  }
  return {}; // std::nullopt
}


std::optional<int32_t> ADS1220::read_single()
{
  int rc = start();
  if( rc != 1 ) {
    return {};
  }
  return read_wait();
}

