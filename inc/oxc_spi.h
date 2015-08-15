#ifndef _OXC_SPI_H
#define _OXC_SPI_H

#include <oxc_gpio.h>
#include <oxc_miscfun.h>


class DevSPI  {
  public:
   enum {
   };
   DevSPI( SPI_HandleTypeDef *a_spi, PinsOut *a_nss_pin = nullptr )
     : spi( a_spi ), nss_pin( a_nss_pin )
    {
    }
   void initSPI();
   void deInit();
   void resetDev();
   SPI_HandleTypeDef* dev() { return spi; };
   void nss_pre();
   void nss_post();
   int  send( const uint8_t *ds, int ns );
   int  send( uint8_t ds );
   int  send2( const uint8_t *ds1, int ns1, const uint8_t *ds2, int ns2 );
   int  recv( uint8_t *dd, int nd );
   int  recv();
   int  send_recv( const uint8_t *ds, int ns, uint8_t *dd, int nd );
   int  send_recv( uint8_t ds, uint8_t *dd, int nd );
   int  duplex( const uint8_t *ds, uint8_t *dd, int nd ); // send 1 bytes more
   void setMaxWait( uint32_t mv ) { maxWait = mv; }
   void setTssDelay( uint32_t t ) { tss_delay_mcs = t; }
   int  getErr() const { return spi->ErrorCode; };
   int  getState() const { return spi->State; };

  protected:
   SPI_HandleTypeDef *spi;
   PinsOut *nss_pin;
   int maxWait = 100;
   bool inv_nss = false;
   uint32_t tss_delay_mcs = 1;
};


#endif
