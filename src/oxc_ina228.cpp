#include <oxc_ina228.h>
#include <oxc_debug1.h>

int32_t INA228::read24cvt( uint8_t reg )
{
  uint32_t v { 0 };
  int n = recv_reg1(  reg, (uint8_t*)(&v), 3 ); // 24bit, 4 low == 0
  if( n == 3 ) {
    v = rev32( v );
    v >>= 12; // always zero + offset 3/4;
    //        000FFFF4
    if( v & 0x00080000 ) {
      v |=  0xFFF80000;
    }
  }
  return (int32_t)v;
}


