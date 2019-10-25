#ifndef _OXC_SPI_H
#define _OXC_SPI_H

#include <oxc_gpio.h>
#include <oxc_miscfun.h>

namespace SPI_lmode {
  enum lmode_enum {
  low_1e = 0,
  high_1e = 1,
  low_2e = 2,
  high_2e = 3
  };
};

int SPI_init_default( uint32_t baud_presc  = SPI_BAUDRATEPRESCALER_256, SPI_lmode::lmode_enum lmode = SPI_lmode::low_1e );
extern SPI_HandleTypeDef spi_h;

class DevSPI  {
  public:
   DevSPI( SPI_HandleTypeDef *a_spi, PinOut *a_nss_pin = nullptr )
     : spi( a_spi ), nss_pin( a_nss_pin )
    {
    }
   void initSPI();
   void deInit();
   void resetDev();
   SPI_HandleTypeDef* dev() { return spi; };
   SPI_TypeDef* instance() { return spi->Instance; }
   void nss_pre();
   void nss_post();
   void nss_pre_cond()  { if( use_nss )  nss_pre();  };
   void nss_post_cond() { if( use_nss )  nss_post(); };
   int  send( const uint8_t *ds, int ns );
   int  send( uint8_t ds );
   int  send2( const uint8_t *ds1, int ns1, const uint8_t *ds2, int ns2 );
   int  recv( uint8_t *dd, int nd );
   int  recv();
   int  send_recv( const uint8_t *ds, int ns, uint8_t *dd, int nd );
   int  send_recv( uint8_t ds, uint8_t *dd, int nd );
   int  duplex( const uint8_t *ds, uint8_t *dd, int nd ); // send 1 bytes more
   void setMaxWait( uint32_t mv ) { maxWait = mv; }
   void setTssDelay( uint32_t t ) { tss_delay_100ns = t * 10; }
   void setTssDelay_100ns( uint32_t t ) { tss_delay_100ns = t; }
   void setUseNss( bool un ) { use_nss = un; } // for send recv duplex
   int  getErr() const { return spi->ErrorCode; };
   int  getLastErr() const { return last_err; };
   int  resetLastErr() { int old_last_err = last_err; last_err = 0; return old_last_err; };
   int  getLastRc() const { return last_rc; };
   int  getState() const { return spi->State; };
   // low-level
   HAL_StatusTypeDef waitForFlag( uint32_t flag, uint32_t val, int ticks = -1 );  // -1 means 'use maxWait'
   HAL_StatusTypeDef waitForTXE( int ticks = -1 )
    { return waitForFlag( SPI_FLAG_TXE, SPI_FLAG_TXE, ticks ); }
   HAL_StatusTypeDef waitForNXRE( int ticks = -1 )
    { return waitForFlag( SPI_FLAG_RXNE, SPI_FLAG_RXNE, ticks ); }
   int  sendSame( uint8_t ds, int ns );
   int  txrx( uint8_t ds, uint8_t *dr = nullptr );
   #ifdef USE_OXC_DEBUG
     void pr_info() const;
   #endif

  protected:
   SPI_HandleTypeDef *spi;
   PinOut *nss_pin;
   int maxWait = 100;
   bool inv_nss = false;
   bool use_nss = true;
   uint32_t tss_delay_100ns = 10;
   uint32_t last_err = 0;
   HAL_StatusTypeDef last_rc = HAL_OK;
};

#ifdef USE_OXC_DEBUG
#include <oxc_console.h>
extern const BitNames SPI_CR1_bitnames[];
extern const BitNames SPI_CR2_bitnames[];
extern const BitNames SPI_SR_bitnames[];
void print_SPI_info( SPI_TypeDef *spi );
#endif

#endif
