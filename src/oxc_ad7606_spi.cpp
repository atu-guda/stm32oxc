#include <oxc_ad7606_spi.h>

using namespace std;

void AD7606_SPI::init()
{
  spi_d.initSPI();
  rst_pin.initHW();
  cnvst_pin.initHW();
  busy_pin.initHW();
  rst_pin.reset( 1 );
  cnvst_pin.set( 1 );
  busy_waited = 0;
}

uint32_t AD7606_SPI::wait_nobusy()
{
  busy_waited = 0;
  // while( ( busy_pin.read() == 0 ) && busy_waited < 10000 ) {
  //   ++busy_waited;
  // }
  while( busy_pin.read() && busy_waited < 10000 ) {
    ++busy_waited;
  }
  return busy_waited;
}

int AD7606_SPI::read_only( int16_t *d, unsigned n )
{
  int16_t buf[8];
  int rc =  spi_d.recv( (uint8_t*)(buf), 2*n );
  for( unsigned i=0; i<n; ++i ) {
    d[i] = __REV16( buf[i] );
  }
  return rc;
}

int AD7606_SPI::read( int16_t *d, unsigned n )
{
  start();
  int rc =  wait_nobusy();
  if( rc > 10000 ) {
    return 0;
  }
  // delay_mcs( 1 ); // ???
  return read_only( d, n );
}

