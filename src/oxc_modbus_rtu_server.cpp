#include <cstring>
#include <iterator>
#include <algorithm>

#include <oxc_auto.h>

#include <oxc_modbus_rtu_server.h>



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

  byte_span sp_i { ModbusRtuWrite1Req::make_span( ibuf ) };
  auto retc = sendReadRepl( sp_o, sp_i );
  return retc;
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

  byte_span sp_i { ModbusRtuReadNRespHead::make_span( ibuf, n ) };
  auto retc = sendReadRepl( sp_o, sp_i );
  if( retc != rcOk ) {
    return retc;
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

ReturnCode MODBUS_RTU_server::sendReadRepl( const byte_span sp_o, byte_span sp_i )
{
  HAL_UART_Receive( uart, (uint8_t*)ibuf, sizeof(ibuf), 0 ); // clear
  auto rc = HAL_UART_Transmit( uart, obuf, sp_o.size(), tout_write );
  if( rc != HAL_OK ) {
    err = errTransmit;
    return rcErr;
  }

  if( obuf[0] == 0 ) { // broadcast = no replay
    return rcOk;
  }
  // BUG: need correct read, but this method works for now
  std::ranges::fill( sp_i, '\0' );
  uint16_t nr;
  rc = HAL_UARTEx_ReceiveToIdle( uart, (uint8_t*)ibuf, sp_i.size(), &nr, tout_read );

  if ( rc != HAL_OK || nr < 2 ) {
    err = errReceive;
    return rcErr;
  }
  if( ibuf[1] & 0x80 ) {
    err = errReplErr; errRepl = ibuf[2];
    return rcErr;
  }

  if( nr != sp_i.size() || !checkRtuCrc( sp_i ) ) {
    err = errCRC;
    return rcErr;
  }
  if( ibuf[0] != obuf[0] ) {
    err = errAddr;
    return rcErr;
  }
  if( ibuf[1] != obuf[1] ) {
    err = errFun;
    return rcErr;
  }
  return rcOk;
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

