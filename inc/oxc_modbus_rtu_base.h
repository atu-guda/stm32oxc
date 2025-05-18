#ifndef _OXC_MODBUS_RTU_BASE_H
#define _OXC_MODBUS_RTU_BASE_H

#include <oxc_cpptypes.h>


// TODO: namespace oxc_modbus

enum class ModbusFunctionCode : uint8_t {
  ReadCoils            =    1,
  ReadDiscreteInputs   =    2,
  ReadHoldingRegs      =    3,
  ReadInputRegs        =    4,
  WriteCoil            =    5,
  WriteSingleReg       =    6,
  ReadExeptStatus      =    7,
  Diagnostics          =    8,
  GetCommEventCount    =   11,
  GetCommEventLog      =   12,
  WriteCoils           =   15,
  WriteNReg            =   16,
  ReportServirID       =   17
};

extern const uint16_t ModbusRtu_CRC_table[256];

uint16_t calcRtuCrc( cbyte_span s );
uint16_t calcRtuCrcSub( cbyte_span s ); // w/o last 2 bytes
void embedRtuCrc( byte_span s );
bool checkRtuCrc( cbyte_span s );

// ==================== packet structures =================================================


struct ModbusRtuReadNReq {
  uint8_t addr;
  ModbusFunctionCode fun;
  uint16_t start;
  uint16_t n;
  uint16_t crc;
  void fill( uint8_t a_addr, uint16_t a_start, uint16_t a_n ) {
    addr = a_addr;  fun = ModbusFunctionCode::ReadHoldingRegs;
    start = rev16( a_start );  n = rev16( a_n );
    // not embedRtuCrc( s );
  }
  // void dump() const;
  static bool make( uint8_t addr, uint16_t start, uint16_t n, byte_span s );
  static bool check( cbyte_span s );
  static byte_span  make_span( uint8_t *buf )        { return { buf, sizeof(ModbusRtuReadNReq) }; };
  static cbyte_span make_cspan( const uint8_t *buf ) { return { buf, sizeof(ModbusRtuReadNReq) }; };
} __attribute__((__packed__));

struct ModbusRtuWrite1Req {
  uint8_t addr;
  ModbusFunctionCode fun;
  uint16_t reg;
  uint16_t val;
  uint16_t crc;
  void fill( uint8_t a_addr, uint16_t a_reg, uint16_t a_val ) {
    addr = a_addr;  fun = ModbusFunctionCode::WriteSingleReg;
    reg = rev16( a_reg );  val = rev16( a_val );
  }
  // void dump() const;
  static bool make( uint8_t addr, uint16_t reg, uint16_t val, byte_span s );
  static bool check( cbyte_span s );
  static byte_span  make_span( uint8_t *buf )        { return { buf, sizeof(ModbusRtuWrite1Req) }; };
  static cbyte_span make_cspan( const uint8_t *buf ) { return { buf, sizeof(ModbusRtuWrite1Req) }; };
} __attribute__((__packed__));

struct ModbusRtuReadNRespHead {
  uint8_t addr;
  ModbusFunctionCode fun;
  uint8_t bytes;
  // data uint16_t * v, crc
  uint8_t get_n() const { return bytes/2; }
  void fill( uint8_t a_addr, uint8_t a_n ) {
    addr = a_addr;  fun = ModbusFunctionCode::ReadHoldingRegs;
    bytes = (uint8_t)a_n*2;
  }
  uint16_t get_v( unsigned i ) const;
  // void dump() const;
  static bool make( uint8_t addr, uint8_t a_n, const uint16_t *vals, byte_span s );
  static bool check( cbyte_span s );
  static byte_span  make_span( uint8_t *buf, uint16_t n )
    { return { buf, sizeof(ModbusRtuReadNRespHead) + sizeof(uint16_t)*(n+1) }; };
  static cbyte_span make_cspan( const uint8_t *buf, uint16_t n )
    { return { buf, sizeof(ModbusRtuReadNRespHead) + sizeof(uint16_t)*(n+1)  }; };
} __attribute__((__packed__));




#endif

