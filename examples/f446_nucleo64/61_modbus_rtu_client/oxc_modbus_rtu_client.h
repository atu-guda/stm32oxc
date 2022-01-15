#ifndef _OXC_MODBUS_RTU_CLIENT_H
#define _OXC_MODBUS_RTU_CLIENT_H

class MODBUS_RTU_client {
  public:
    enum MODBUS_client_state {
      ST_INIT = 0,
      ST_IDLE = 1,
      ST_RECV = 2,
      ST_ERR  = 3
    };
    static const uint16_t bufsz = 256;
    MODBUS_RTU_client();
    static uint16_t crc( const uint8_t *s, uint16_t l );
    const uint8_t* get_ibuf() const { return ibuf; }
    const uint8_t* get_obuf() const { return obuf; }
  private:
    static const uint8_t CRC_hi[256];
    static const uint8_t CRC_lo[256];

    uint8_t ibuf[bufsz];
    uint8_t obuf[bufsz];
    uint16_t i_pos = 0, o_pos = 0;
};



#endif

