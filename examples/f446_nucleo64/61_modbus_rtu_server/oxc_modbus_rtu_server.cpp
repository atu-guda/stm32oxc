#include <cstring>
#include <iterator>
#include <algorithm>

#include <oxc_auto.h>

#include <oxc_modbus_rtu_server.h>


// ================================== common ModbusRtu ========================
// TODO: separate file, may be used with client

uint16_t calcRtuCrc( cbyte_span s )
{
  uint16_t  u { 0xFFFF };

  for( auto c : s ) {
    uint8_t t = c ^ u;
    u >>= 8;
    u ^=  ModbusRtu_CRC_table[t];
  }
  return u;
}

uint16_t calcRtuCrcSub( cbyte_span s )
{
  return calcRtuCrc( s.subspan( 0, s.size()-2 ) );
}

bool checkRtuCrc( cbyte_span s )
{
  if( s.size() < 3 ) {
    return false;
  }
  return ( *std::bit_cast<uint16_t*>( s.data() + ( s.size()-2 ) ) ) == calcRtuCrcSub( s );
}


void embedRtuCrc( byte_span s )
{
  if( s.size() < 3 ) {
    return;
  }
  *std::bit_cast<uint16_t*>( s.data() + ( s.size()-2 ) ) = calcRtuCrcSub( s );
}

const uint16_t ModbusRtu_CRC_table[256] {
  0x0000, 0xC0C1, 0xC181, 0x0140,  0xC301, 0x03C0, 0x0280, 0xC241,
  0xC601, 0x06C0, 0x0780, 0xC741,  0x0500, 0xC5C1, 0xC481, 0x0440,
  0xCC01, 0x0CC0, 0x0D80, 0xCD41,  0x0F00, 0xCFC1, 0xCE81, 0x0E40,
  0x0A00, 0xCAC1, 0xCB81, 0x0B40,  0xC901, 0x09C0, 0x0880, 0xC841,
  0xD801, 0x18C0, 0x1980, 0xD941,  0x1B00, 0xDBC1, 0xDA81, 0x1A40,
  0x1E00, 0xDEC1, 0xDF81, 0x1F40,  0xDD01, 0x1DC0, 0x1C80, 0xDC41,
  0x1400, 0xD4C1, 0xD581, 0x1540,  0xD701, 0x17C0, 0x1680, 0xD641,
  0xD201, 0x12C0, 0x1380, 0xD341,  0x1100, 0xD1C1, 0xD081, 0x1040,
  0xF001, 0x30C0, 0x3180, 0xF141,  0x3300, 0xF3C1, 0xF281, 0x3240,
  0x3600, 0xF6C1, 0xF781, 0x3740,  0xF501, 0x35C0, 0x3480, 0xF441,
  0x3C00, 0xFCC1, 0xFD81, 0x3D40,  0xFF01, 0x3FC0, 0x3E80, 0xFE41,
  0xFA01, 0x3AC0, 0x3B80, 0xFB41,  0x3900, 0xF9C1, 0xF881, 0x3840,
  0x2800, 0xE8C1, 0xE981, 0x2940,  0xEB01, 0x2BC0, 0x2A80, 0xEA41,
  0xEE01, 0x2EC0, 0x2F80, 0xEF41,  0x2D00, 0xEDC1, 0xEC81, 0x2C40,
  0xE401, 0x24C0, 0x2580, 0xE541,  0x2700, 0xE7C1, 0xE681, 0x2640,
  0x2200, 0xE2C1, 0xE381, 0x2340,  0xE101, 0x21C0, 0x2080, 0xE041,
  0xA001, 0x60C0, 0x6180, 0xA141,  0x6300, 0xA3C1, 0xA281, 0x6240,
  0x6600, 0xA6C1, 0xA781, 0x6740,  0xA501, 0x65C0, 0x6480, 0xA441,
  0x6C00, 0xACC1, 0xAD81, 0x6D40,  0xAF01, 0x6FC0, 0x6E80, 0xAE41,
  0xAA01, 0x6AC0, 0x6B80, 0xAB41,  0x6900, 0xA9C1, 0xA881, 0x6840,
  0x7800, 0xB8C1, 0xB981, 0x7940,  0xBB01, 0x7BC0, 0x7A80, 0xBA41,
  0xBE01, 0x7EC0, 0x7F80, 0xBF41,  0x7D00, 0xBDC1, 0xBC81, 0x7C40,
  0xB401, 0x74C0, 0x7580, 0xB541,  0x7700, 0xB7C1, 0xB681, 0x7640,
  0x7200, 0xB2C1, 0xB381, 0x7340,  0xB101, 0x71C0, 0x7080, 0xB041,
  0x5000, 0x90C1, 0x9181, 0x5140,  0x9301, 0x53C0, 0x5280, 0x9241,
  0x9601, 0x56C0, 0x5780, 0x9741,  0x5500, 0x95C1, 0x9481, 0x5440,
  0x9C01, 0x5CC0, 0x5D80, 0x9D41,  0x5F00, 0x9FC1, 0x9E81, 0x5E40,
  0x5A00, 0x9AC1, 0x9B81, 0x5B40,  0x9901, 0x59C0, 0x5880, 0x9841,
  0x8801, 0x48C0, 0x4980, 0x8941,  0x4B00, 0x8BC1, 0x8A81, 0x4A40,
  0x4E00, 0x8EC1, 0x8F81, 0x4F40,  0x8D01, 0x4DC0, 0x4C80, 0x8C41,
  0x4400, 0x84C1, 0x8581, 0x4540,  0x8701, 0x47C0, 0x4680, 0x8641,
  0x8201, 0x42C0, 0x4380, 0x8341,  0x4100, 0x81C1, 0x8081, 0x4040
};



// ==================== packet structures =================================================


bool ModbusRtuReadNReq::check( cbyte_span s )
{
  if( s.size() != sizeof(ModbusRtuReadNReq) ) {
    return false;
  }
  if( ! checkRtuCrc( s ) ) {
    return false;
  }

  auto r = std::bit_cast<ModbusRtuReadNReq *>(s.data());
  return r->fun == ModbusFunctionCode::ReadHoldingRegs;
}

bool ModbusRtuReadNReq::make( uint8_t addr, uint16_t start, uint16_t n, byte_span s )
{
  if( s.size() != sizeof(ModbusRtuReadNReq) ) {
    return false;
  }
  auto r = std::bit_cast<ModbusRtuReadNReq *>(s.data());
  r->fill( addr, start, n );
  embedRtuCrc( s );
  return true;
}


bool ModbusRtuWrite1Req::check( cbyte_span s )
{
  if( s.size() != sizeof(ModbusRtuWrite1Req) ) {
    return false;
  }
  if( ! checkRtuCrc( s ) ) {
    return false;
  }

  auto r = std::bit_cast<ModbusRtuWrite1Req *>(s.data());
  return r->fun == ModbusFunctionCode::WriteSingleReg;
}

bool ModbusRtuWrite1Req::make( uint8_t addr, uint16_t reg, uint16_t val, byte_span s )
{
  if( s.size() != sizeof(ModbusRtuWrite1Req) ) {
    return false;
  }
  auto r = std::bit_cast<ModbusRtuWrite1Req*>(s.data());
  r->fill( addr, reg, val );
  embedRtuCrc( s );
  return true;
}


bool ModbusRtuReadNRespHead::check( cbyte_span s )
{
  if( s.size() < sizeof(ModbusRtuReadNRespHead) + sizeof(uint16_t) ) {
    // cout << "Err: size " << s.size() << endl;
    return false;
  }
  if( ! checkRtuCrc( s ) ) {
    // cout << "Err: crc " << s.size() << endl;
    return false;
  }

  auto r = std::bit_cast<ModbusRtuWrite1Req *>(s.data());
  return r->fun == ModbusFunctionCode::ReadHoldingRegs;
}

bool ModbusRtuReadNRespHead::make( uint8_t addr, uint8_t a_n, const uint16_t * /*vals*/, byte_span s )
{
  if( s.size() != sizeof(ModbusRtuReadNRespHead) ) {
    return false;
  }
  auto r = std::bit_cast<ModbusRtuReadNRespHead*>(s.data());
  r->fill( addr, a_n );
  embedRtuCrc( s );
  return true;
}

uint16_t ModbusRtuReadNRespHead::get_v( unsigned i ) const
{
  if( i >= get_n() ) {
    return 0;
  }
  const uint16_t *pd = std::bit_cast<const uint16_t *>( &bytes  + sizeof(uint8_t) );
  return rev16( pd[i] );
}


// ================================================= server ===============================

MODBUS_RTU_server::MODBUS_RTU_server( UART_HandleTypeDef *a_uart )
  : uart( a_uart )
{
  reset();
}

void MODBUS_RTU_server::reset()
{
  // state = ST_IDLE;
  last_uart_status = 0;
  n_readed_regs = 0; start_reg = 0;
  err = errOk;  errRepl = 0;
  std::ranges::fill( ibuf, '\x00' );
  std::ranges::fill( obuf, '\x00' );
}


ReturnCode MODBUS_RTU_server::writeReg( uint8_t addr, uint16_t reg, uint16_t val )
{
  err = errOk;  errRepl = 0;
  byte_span sp_o { ModbusRtuWrite1Req::make_span( obuf ) };
  auto rc_b = ModbusRtuWrite1Req::make( addr, reg, val, sp_o );
  if( !rc_b ) {
    err = errBadParam;
    return rcErr;
  }
  n_readed_regs = 0; start_reg = 0;

  // dump8( obuf, sizeof(ModbusRtuWrite1Req) );

  HAL_UART_Receive( uart, (uint8_t*)ibuf, sizeof(ibuf), 0 ); // clear
  auto rc = HAL_UART_Transmit( uart, obuf, sizeof(ModbusRtuWrite1Req), tout_write );
  if( rc != HAL_OK ) {
    err = errTransmit;
    return rcErr;
  }

  if( addr == 0 ) { // broadcast = no replay
    return rcOk;
  }

  // BUG: need correct read, but this method works for now
  byte_span sp_i { ModbusRtuWrite1Req::make_span( ibuf ) };
  std::ranges::fill( sp_i, '\0' );
  rc = HAL_UART_Receive( uart, (uint8_t*)ibuf, sizeof(ModbusRtuWrite1Req), tout_read );

  // dump8( ibuf, sizeof(ModbusRtuWrite1Req) );

  if ( rc != HAL_OK ) {
    err = errReceive; errRepl = ibuf[2]; // as bad rc handling
    return rcErr;
  }
  if( !checkRtuCrc( sp_i ) ) {
    err = errCRC;
    return rcErr;
  }

  if( ibuf[0] != addr ) {
    err = errAddr;
    return rcErr;
  }
  if( ibuf[1] & 0x80 ) {
    err = errReplErr; errRepl = ibuf[2];
    return rcErr;
  }
  if( ibuf[1] != (uint8_t)ModbusFunctionCode::WriteSingleReg ) {
    err = errFun;
    return rcErr;
  }
  return rcOk;
}

ReturnCode MODBUS_RTU_server::readRegs( uint8_t addr, uint16_t start, uint16_t n )
{
  err = errOk;  errRepl = 0;
  if( addr < 1 ||  n > 125 || ((unsigned)start+n) > 0xFFFE ) {
    err = errBadParam;
    return rcErr;
  }
  n_readed_regs = 0; start_reg = 0;

  byte_span sp_o { ModbusRtuReadNReq::make_span( obuf ) };
  auto rc_b = ModbusRtuReadNReq::make( addr, start, n, sp_o );
  if( !rc_b ) {
    err = errBadParam;
    return rcErr;
  }

  // dump8( obuf, sizeof(ModbusRtuReadNReq) );

  HAL_UART_Receive( uart, (uint8_t*)ibuf, sizeof(ibuf), 0 ); // clear
  auto rc = HAL_UART_Transmit( uart, obuf, sizeof(ModbusRtuReadNReq), tout_write );
  if( rc != HAL_OK ) {
    err = errTransmit;
    return rcErr;
  }

  // BUG: need correct read, but this method works for now
  byte_span sp_i { ModbusRtuReadNRespHead::make_span( ibuf, n ) };
  std::ranges::fill( sp_i, '\0' );
  rc = HAL_UART_Receive( uart, (uint8_t*)ibuf, sp_i.size(), tout_read );

  // dump8( ibuf, sp_o.size() );

  if ( rc != HAL_OK ) {
    err = errReceive; errRepl = ibuf[2]; // as bad rc handling
    return rcErr;
  }
  if( !checkRtuCrc( sp_i ) ) {
    err = errCRC;
    return rcErr;
  }
  if( ibuf[0] != addr ) {
    err = errAddr;
    return rcErr;
  }
  if( ibuf[1] & 0x80 ) {
    err = errReplErr; errRepl = ibuf[2];
    return rcErr;
  }
  if( ibuf[1] != (uint8_t)ModbusFunctionCode::ReadHoldingRegs ) {
    err = errFun;
    return rcErr;
  }
  n_readed_regs = n; start_reg = start;
  return rcOk;
}

uint16_t MODBUS_RTU_server::getReg( uint16_t i ) const
{
  if( i < start_reg ) {
    return 0;
  }
  i -= start_reg;
  if( i >= n_readed_regs ) {
    return 0;
  }
  auto vs = std::bit_cast<uint16_t *>( ibuf+sizeof(ModbusRtuReadNRespHead) );
  return rev16( vs[i] );
}

std::expected<uint16_t,ReturnCode> MODBUS_RTU_server::readGetReg( uint8_t addr, uint16_t i )
{
  auto rc = readRegs( addr, i, 1 );
  if( rc != rcOk ) {
    return std::unexpected{ rc };
  }
  auto vs = std::bit_cast<uint16_t *>( ibuf+sizeof(ModbusRtuReadNRespHead) );
  return rev16( *vs );
}

