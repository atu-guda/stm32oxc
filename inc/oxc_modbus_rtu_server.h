#ifndef _OXC_MODBUS_RTU_SERVER_H
#define _OXC_MODBUS_RTU_SERVER_H

#include <oxc_modbus_rtu_base.h>


// ================================================= server ===============================

class MODBUS_RTU_server {
  public:
    enum timeouts {
      tout_write = 100,
      tout_read  = 500
    };
    enum Errors {
      errOk = 0,
      errBadParam = 1,
      errTransmit = 2,
      errReceive  = 3,
      errCRC      = 4,
      errReplErr  = 5, // modbus error responce
      errAddr     = 6, // wrong replay address
      errFun      = 7, // wrong func number, but not error

    };
    static const uint16_t ibufsz { 256 };
    static const uint16_t obufsz {  32 }; // as w do not write N regs
    explicit MODBUS_RTU_server( UART_HandleTypeDef *a_uart ); // TODO: oxc?
    const uint8_t* get_ibuf() const { return ibuf; }
    const uint8_t* get_obuf() const { return obuf; }
    uint32_t get_last_uart_status() const { return last_uart_status; }
    // server_state get_server_state() const { return state; }
    void reset();
    ReturnCode writeReg( uint8_t addr, uint16_t reg, uint16_t val );
    ReturnCode readRegs( uint8_t addr, uint16_t start, uint16_t n );
    ReturnCode writeReg_ntry( uint8_t addr, uint16_t reg, uint16_t val );
    ReturnCode readRegs_ntry( uint8_t addr, uint16_t start, uint16_t n );
    uint16_t getNReadedRegs() const { return n_readed_regs; }
    uint16_t getReg( uint16_t i ) const;
    std::expected<uint16_t,ReturnCode> readGetReg( uint8_t addr, uint16_t i );
    std::expected<uint16_t,ReturnCode> readGetReg_ntry( uint8_t addr, uint16_t i );
    Errors getError() const { return err; }
    uint8_t getReplError() const { return errRepl; }
    uint32_t get_n_try() const  { return n_try; }
    void set_n_try( uint32_t n ) { n_try = n; }
    uint32_t get_t_try() const  { return t_try; }
    void set_t_try( uint32_t t ) { t_try = t; }

  private:
    // ReturnCode readRepl( byte_span sp_i );
    ReturnCode sendReadRepl( const byte_span sp_o, byte_span sp_i ); // common actions on writeReg, readRegs
    uint8_t ibuf[ibufsz];
    uint8_t obuf[obufsz];
    UART_HandleTypeDef *uart;
    // server_state state = ST_INIT;
    uint32_t last_uart_status { 0 };
    uint32_t n_try { 10 };
    uint32_t t_try { 100 };
    uint16_t n_readed_regs { 0 };
    uint16_t start_reg { 0 };
    Errors err { errOk };
    uint8_t errRepl { 0 };

};



#endif

