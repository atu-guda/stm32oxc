#ifndef _OXC_I2C_H
#define _OXC_I2C_H

#include <oxc_miscfun.h>


class DevI2C  {
  public:
   enum {
     default_i2c_speed = 100000,
     max_i2c_speed     = 400000
   };
   DevI2C( I2C_HandleTypeDef *a_i2ch, uint8_t d_addr  ) // addr is only default
     : i2ch( a_i2ch ), addr2(  d_addr << 1 )
    {
    }
   void initI2C( uint32_t speed = default_i2c_speed, uint8_t own_addr = 0 );
   void deInit();
   void resetDev();
   void setAddr( uint8_t addr ) { addr2 = addr << 1; }
   uint8_t getAddr() const { return addr2>>1; }
   uint8_t effAddr( uint8_t addr ) const  { return (addr!=0) ? (addr<<1) : addr2; };
   I2C_HandleTypeDef* dev() { return i2ch; };
   bool  ping( uint8_t addr = 0 ); // 0 means addr2
   // returns: >0 = N of send/recv bytes, <0 - error
   int  send( uint8_t ds, uint8_t addr = 0 );
   int  send( const uint8_t *ds, int ns, uint8_t addr = 0 );
   int  send_reg12(  uint8_t reg,  const uint8_t *ds, int ns, bool is2byte, uint8_t addr = 0 );
   int  send_reg1(  uint8_t reg,  const uint8_t *ds, int ns, uint8_t addr = 0  );
   int  send_reg1(  uint8_t reg,  uint8_t d, uint8_t addr = 0 )
     { return send_reg1( reg, &d, 1, addr ); }
   int  send_reg2(  uint16_t reg, const uint8_t *ds, int ns, uint8_t addr = 0 );
   int  send_reg2(  uint8_t reg,  uint8_t d, uint8_t addr = 0 )
     { return send_reg2( reg, &d, 1, addr ); }
   int  recv( uint8_t addr = 0 );
   int  recv( uint8_t *dd, int nd, uint8_t addr = 0 );
   int  recv_reg12(  int8_t reg,  uint8_t *dd, int nd, bool is2byte, uint8_t addr = 0 );
   int  recv_reg1(  int8_t reg,  uint8_t *dd, int nd, uint8_t addr = 0 );
   int  recv_reg2(  int16_t reg, uint8_t *dd, int nd, uint8_t addr = 0 );
   void setMaxWait( uint32_t mv ) { maxWait = mv; }
   int  getErr() const { return i2ch->ErrorCode; };
   int  getState() const { return i2ch->State; };

  protected:
   I2C_HandleTypeDef *i2ch;
   uint8_t addr2;
   int maxWait = 100;
};

class I2CClient {
  public:
   I2CClient( DevI2C &a_dev, uint8_t d_addr )
     : dev( a_dev ), addr( d_addr ) {};
   void setAddr( uint8_t d_addr ) { addr = d_addr; };
   uint8_t getAddr() const { return addr; }
   void resetDev() { dev.resetDev(); }
   int  send( uint8_t ds )
      {  return dev.send( ds, addr ); };
   int  send( const uint8_t *ds, int ns )
      { return dev.send( ds, ns, addr ); }
   int  send_reg12(  uint8_t reg,  const uint8_t *ds, int ns, bool is2byte )
      { return dev.send_reg12( reg, ds, ns, is2byte, addr ); };
   int  send_reg1(  uint8_t reg,  const uint8_t *ds, int ns )
      { return dev.send_reg1( reg, ds, ns, addr ); };
   int  send_reg1(  uint8_t reg,  uint8_t d )
      { return dev.send_reg1( reg, d, addr ); }
   int  send_reg2(  uint16_t reg, const uint8_t *ds, int ns )
     { return dev.send_reg2( reg, ds, ns, addr ); }
   int  send_reg2(  uint8_t reg,  uint8_t d )
     { return dev.send_reg2( reg, d, addr ); }
   int  recv()
     { return dev.recv( addr ); }
   int  recv( uint8_t *dd, int nd )
     { return dev.recv( dd, nd, addr ); }
   int  recv_reg12(  int8_t reg,  uint8_t *dd, int nd, bool is2byte )
     { return dev.recv_reg12( reg, dd, nd, is2byte, addr ); }
   int  recv_reg1(  int8_t reg,  uint8_t *dd, int nd )
     { return dev.recv_reg1( reg, dd, nd, addr ); }
   int  recv_reg2(  int16_t reg, uint8_t *dd, int nd )
     { return dev.recv_reg2( reg, dd, nd, addr ); }
   int  getErr() const { return dev.getErr(); };
   int  getState() const { return dev.getState(); };
  protected:
   DevI2C &dev;
   uint8_t addr;
};

#endif
