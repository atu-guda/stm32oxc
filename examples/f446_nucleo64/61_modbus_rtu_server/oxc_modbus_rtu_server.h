#ifndef _OXC_MODBUS_RTU_SERVER_H
#define _OXC_MODBUS_RTU_SERVER_H

class MODBUS_RTU_server {
  public:
    enum server_state {
      ST_INIT   = 0, // ? unused?
      ST_IDLE   = 1,
      ST_RECV   = 2,
      ST_MSG_IN = 3,
      ST_ERR    = 4
    };
    static const uint16_t bufsz = 256;
    MODBUS_RTU_server( USART_TypeDef *a_uart, volatile uint32_t *a_tim_cnt );
    static uint16_t crc( const uint8_t *s, uint16_t l );
    bool isCrcGood() const;
    const uint8_t* get_ibuf() const { return ibuf; }
    const uint8_t* get_obuf() const { return obuf; }
    uint16_t get_ibuf_pos() const { return i_pos; }
    uint16_t get_obuf_pos() const { return o_pos; }
    uint32_t get_last_uart_status() const { return last_uart_status; }
    server_state get_server_state() const { return state; }
    void reset();
    void handle_UART_IRQ();
    void handle_tick();
  private:
    static const uint8_t CRC_hi[256];
    static const uint8_t CRC_lo[256];

    uint8_t  ibuf[bufsz];
    uint8_t obuf[bufsz];
    USART_TypeDef *uart;
    volatile uint32_t *tim_cnt;
    uint16_t i_pos = 0, o_pos = 0;
    uint16_t t_char = 0;
    server_state state = ST_INIT;
    uint32_t last_uart_status = 0;

};



#endif

