#include <oxc_spimem_at.h>

uint8_t DevSPIMem_AT::status()
{
  uint8_t r = 0;
  spi.send_recv( (uint8_t)(Cmd::RDSR), (uint8_t*)(&r), 1 );
  return r;
}

bool DevSPIMem_AT::write_status( uint8_t sta )
{
  uint8_t c[2];
  c[0] = Cmd::WRSR; c[1] = sta;
  int n = spi.send( c, 2 );
  return (n>0);
}

int DevSPIMem_AT::write( const uint8_t *buf, uint32_t addr, int n )
{
  write_enable();
  uint8_t c[4];
  c[0] = Cmd::PGM; c[3] = addr & 0xFF;
  addr <<= 8;      c[2] = addr & 0xFF;
  addr <<= 8;      c[1] = addr & 0xFF;
  int nt = spi.send2( c, 4, buf, n );
  bool r = wait_ready();
  return r ? nt : 0;
}

int DevSPIMem_AT::read( uint8_t *buf, uint32_t addr, int n )
{
  uint8_t c[4];
  c[0] = Cmd::READ;c[3] = addr & 0xFF;
  addr <<= 8;      c[2] = addr & 0xFF;
  addr <<= 8;      c[1] = addr & 0xFF;
  int nt = spi.send_recv( c, 4, buf, n );
  return nt;
}

bool DevSPIMem_AT::write_enable()
{
  return spi.send( Cmd::WREN ) > 0;
}

bool DevSPIMem_AT::write_disable()
{
  return spi.send( Cmd::WRDI ) > 0;
}

uint16_t DevSPIMem_AT::read_id()
{
  uint16_t id;
  spi.send_recv( Cmd::RDID, (uint8_t*)(&id), sizeof(id) );
  return id;
}

bool DevSPIMem_AT::erase_sector( uint32_t addr )
{
  write_enable();
  uint8_t c[4];
  c[0] = Cmd::ER_S;c[3] = addr & 0xFF;
  addr <<= 8;      c[2] = addr & 0xFF;
  addr <<= 8;      c[1] = addr & 0xFF;
  addr <<= 8;
  spi.send( c, 4 );
  bool r = wait_ready();
  return r;
}

bool DevSPIMem_AT::erase_chip()
{
  write_enable();
  spi.send( Cmd::ER_C );
  bool r = wait_ready();
  return r;
}


bool DevSPIMem_AT::wait_ready()
{
  for( int i=0; i<w_max; ++i ) {
    volatile uint8_t s = status();
    if( ! ( s & sflags::BUSY ) ) {
      return true;
    }
    delay_ms( w_quant );
  }

  return false;
}


