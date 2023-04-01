#ifndef _OXC_TMC2209_H
#define _OXC_TMC2209_H

#include <cstdint>
#include <bit>

namespace TMC2209 {

inline const uint32_t bad_val = 0xFFFFFFFF;

uint8_t calc_crc( const uint8_t *d, unsigned sz  );

// TODO: register names

// Structure to write data to TMC2209 and to receive replay from it
struct rwdata {
  uint8_t  sync;
  uint8_t  addr;
  uint8_t  regnum;
  uint32_t data;
  uint8_t  crc;
  //
  void fill( uint8_t dev_addr, uint8_t reg, uint32_t dat );
  inline uint8_t* rawData() {
    return std::bit_cast<uint8_t*>( this );
  }
  inline const uint8_t* rawCData() const {
    return std::bit_cast<const uint8_t*>( this );
  }
} __attribute__((packed));
static_assert( sizeof(rwdata) == 8, "Bad TMC2209::rwdata size" );



// Structure to requiest data from TMC2209
struct rreq {
  uint8_t  sync;
  uint8_t  addr;
  uint8_t  regnum;
  uint8_t  crc;
  void fill( uint8_t dev_addr, uint8_t reg );
  inline uint8_t* rawData() {
    return std::bit_cast<uint8_t*>( this );
  }
  inline const uint8_t* rawCData() const {
    return std::bit_cast<const uint8_t*>( this );
  }
} __attribute__((packed));
static_assert( sizeof(rreq) == 4, "Bad TMC2209::rreq size" );

}; // namespace TMC2209

#endif

