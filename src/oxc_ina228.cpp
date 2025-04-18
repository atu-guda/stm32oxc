#include <oxc_ina228.h>

// returns 0 if good and VID:PID if bad
uint32_t INA228::isBad()
{
  uint16_t id_manuf = readReg( INA228::reg_id_manuf );
  uint16_t id_dev   = readReg( INA228::reg_id_dev );
  if( id_manuf == INA228::id_manuf && id_dev == INA228::id_dev ) {
    return 0;
  }
  return (id_manuf<<16) | id_dev;
}

int32_t INA228::read24cvt( uint8_t reg )
{
  uint32_t v { 0 };
  int n = recv_reg1(  reg, (uint8_t*)(&v), 3 ); // 24bit, 4 low == 0
  if( n == 3 ) {
    v = rev32( v );
    v >>= 12; // always zero + offset 3/4;
    if( v & 0x00080000 ) {
      v |=  0xFFF80000;
    }
  }
  return (int32_t)v;
}

// returs: 0: ok, 1-overtime, 2-break.
int INA228::waitEOC( int max_wait )
{
  for( int i=0; i<max_wait && !break_flag; ++i ) {
    if( getDiag() & 0x02 ) {
      return 0;
    }
    delay_ms( 1 );
  }
  return break_flag ? 2 : 1;
}
