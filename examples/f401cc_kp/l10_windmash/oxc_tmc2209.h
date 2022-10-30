#ifndef _OXC_TMC2209_H
#define _OXC_TMC2209_H

#include <cstdint>
#include <bit>

inline const uint32_t TMC2209_bad_val = 0xFFFFFFFF;

uint8_t TMC22xx_calc_crc( const uint8_t *d, unsigned sz  );

// Structure to write data to TMC2209 and to receive replay from it
struct TMC2209_rwdata {
  uint8_t  sync;
  uint8_t  addr;
  uint8_t  regnum;
  uint32_t data;
  uint8_t  crc;
  //
  void fill( uint8_t dev_addr, uint8_t reg, uint32_t dat );
  inline uint8_t* rawData() { return std::bit_cast<uint8_t*>( this ); }
  inline const uint8_t* rawCData() const { return std::bit_cast<uint8_t*>( this ); }
} __attribute((packed));
static_assert( sizeof(TMC2209_rwdata) == 8, "Bad TMC2209_rwdata size" );



// Structure to requiest data from TMC2209
struct TMC2209_rreq {
  uint8_t  sync;
  uint8_t  addr;
  uint8_t  regnum;
  uint8_t  crc;
  void fill( uint8_t dev_addr, uint8_t reg );
  inline uint8_t* rawData() { return std::bit_cast<uint8_t*>( this ); }
  inline const uint8_t* rawCData() const { return std::bit_cast<uint8_t*>( this ); }
} __attribute((packed));
static_assert( sizeof(TMC2209_rreq) == 4, "Bad TMC2209_rreq size" );

#endif

