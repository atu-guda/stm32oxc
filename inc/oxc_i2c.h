#ifndef _OXC_I2C_H
#define _OXC_I2C_H

#include <oxc_miscfun.h>


class DevI2C  {
  public:
   enum {
     default_i2c_speed = 100000,
     max_i2c_speed     = 400000
   };
   DevI2C( I2C_HandleTypeDef *a_i2ch, uint8_t d_addr  )
     : i2ch( a_i2ch ), addr2(  d_addr << 1 )
    {
    }
   void initI2C( uint32_t speed = default_i2c_speed, uint8_t own_addr = 0 );
   void deInit();
   void resetDev();
   I2C_HandleTypeDef* dev() { return i2ch; };
   bool  ping( uint8_t addr = 0 ); // 0 means addr2
   // returns: >0 = N of send/recv bytes, <0 - error
   int  send( uint8_t ds );
   int  send( const uint8_t *ds, int ns );
   int  send_reg1(  uint8_t reg,  const uint8_t *ds, int ns );
   int  send_reg2(  uint16_t reg, const uint8_t *ds, int ns );
   int  recv();
   int  recv( uint8_t *dd, int nd );
   int  recv_reg1(  int8_t reg,  uint8_t *dd, int nd );
   int  recv_reg2(  int16_t reg, uint8_t *dd, int nd );
   void setMaxWait( uint32_t mv ) { maxWait = mv; }
   int  getErr() const { return i2ch->ErrorCode; };
   int  getState() const { return i2ch->State; };

  protected:
   I2C_HandleTypeDef *i2ch;
   uint8_t addr2;
   int maxWait = 100;
};


#endif
