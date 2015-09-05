#ifndef _OXC_SPI_SDCARD_H
#define _OXC_SPI_SDCARD_H


#include <oxc_spi.h>

class DevSPI_SdCard {
  public:
   enum {
     CAP_V2_00 = 1,
     CAP_SDHC  = 2,
     CRC7_INIT = 0x89
   };

   DevSPI_SdCard( DevSPI &a_spi )
     : spi( a_spi ) {};
   bool isInited() const { return inited; }
   uint32_t getCaps() const { return caps; }
   static uint8_t crc7( uint8_t t, uint8_t d );
   static uint8_t crc7( const uint8_t *p, uint16_t l );
   static uint16_t crc16_ccitt( uint16_t crc, uint8_t d );
   static uint16_t crc16( const uint8_t *p, uint16_t l );
   int sd_cmd( uint8_t c, uint32_t arg );
   uint8_t getR1();
   uint16_t getR2();
   uint8_t getR7( uint32_t *r7 );
   void nec(); // is really need?
   int init();


  protected:
   DevSPI &spi;
   bool inited = false;
   uint32_t caps = 0;
   uint32_t sects = 0;
   uint32_t sects_era = 0;
   uint32_t n_try = 1024;
};


#endif

