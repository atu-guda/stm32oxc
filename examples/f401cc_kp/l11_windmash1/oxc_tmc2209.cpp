#include <cstring>
#include <bit>

#include "oxc_base.h"
#include "oxc_tmc2209.h"

uint8_t TMC2209::calc_crc( const uint8_t *d, unsigned sz )
{
  uint8_t crc = 0;
  for( unsigned i=0; i<sz; ++i ) {
    uint8_t c = d[i];
    for( uint8_t j=0; j<8; ++j ) {
      if( ( crc >> 7) ^ ( c & 0x01 ) ) {
        crc = ( crc << 1 ) ^ 0x07;
      } else {
        crc = ( crc << 1 );
      }
      c >>= 1;
    }
  }
  return crc;
}


void TMC2209::rwdata::fill( uint8_t dev_addr, uint8_t reg, uint32_t dat )
{
  sync = 0x05;
  addr = dev_addr;
  regnum = reg | 0x80;
  data = __builtin_bswap32( dat );
  crc = calc_crc( rawCData(), sizeof(*this)-1 );
}

void TMC2209::rreq::fill( uint8_t dev_addr, uint8_t reg )
{
  sync = 0x05;
  addr = dev_addr;
  regnum = reg;
  crc = calc_crc( rawCData(), sizeof(*this)-1  );
}

uint32_t TMC2209::TMC_devices::read_reg_1( uint8_t dev, uint8_t reg )
{
  TMC2209::rreq  rqd;
  rqd.fill( dev, reg );

  drv->reset();

  // ledsx.set( 1 );

  auto w_n = drv->write( rqd.rawCData(), sizeof(rqd) );
  if( w_n != sizeof(rqd) ) {
    // std_out << "# Err: w_n = " << w_n << NL;
    return TMC2209::bad_val;
  }

  delay_ms( 1 ); // TODO: config

  memset( buf, '\x00', sizeof(buf) );

  auto r_n = drv->read( buf, sizeof(buf) );

  if( r_n != sizeof(TMC2209::rreq) + sizeof(TMC2209::rwdata) ) {
    // if( debug > 0 ) {
    //   std_out << "# Err: 12 != r_n = " << r_n << NL;
    //   dump8( buf, 16 );
    // }
    return TMC2209::bad_val;
  }

  TMC2209::rwdata *rd = std::bit_cast<TMC2209::rwdata*>( buf + sizeof(TMC2209::rreq) );
  // TODO: check crc
  uint32_t v = __builtin_bswap32( rd->data );
  delay_mcs( 100 );
  return v;
}

uint32_t TMC2209::TMC_devices::read_reg( uint8_t dev, uint8_t reg )
{
  for( decltype(+try_max) i=0; i < try_max; ++i ) {
    uint32_t v = read_reg_1( dev, reg );
    if( v != TMC2209::bad_val ) {
      return v;
    }
    delay_ms( wait_max );
  }
  return TMC2209::bad_val;
}

int TMC2209::TMC_devices::write_reg_1( uint8_t dev, uint8_t reg, uint32_t v )
{
  TMC2209::rwdata wd;
  wd.fill( dev, reg, v );
  auto w_n = drv->write( wd.rawCData(), sizeof(wd) );
  if( w_n != sizeof(wd) ) {
    return 0;
  }
  return 1;
}

int TMC2209::TMC_devices::write_reg( uint8_t dev, uint8_t reg, uint32_t v )
{
  for( decltype(+try_max) i=0; i < try_max; ++i ) {
    uint32_t cnt0 = read_reg( dev, 2 ); // TODO: reg number
    if( cnt0 == TMC2209::bad_val ) {
      return TMC2209::bad_val;
    }
    auto rc = write_reg_1( dev, reg, v );
    if( rc == (int)TMC2209::bad_val ) {
      return TMC2209::bad_val;
    }
    uint32_t cnt1 = read_reg( dev, 2 ); // TODO: reg number
    if( cnt1 == TMC2209::bad_val ) {
      return TMC2209::bad_val;
    }
    if( ( cnt1 - cnt0 ) == 1 ) {
      return 1;
    }

    delay_ms( wait_max );
  }
  return TMC2209::bad_val;
}

