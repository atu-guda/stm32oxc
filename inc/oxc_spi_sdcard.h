#ifndef _OXC_SPI_SDCARD_H
#define _OXC_SPI_SDCARD_H


#include <oxc_spi.h>

class DevSPI_SdCard {
  public:
   enum {
     CAP_V2_00 = 1,
     CAP_SDHC  = 2,
     CRC7_INIT = 0x89,
     ERR_SPI = 1,
     ERR_BITS = 2,
     ERR_TOUT = 4,
     ERR_CRC = 8
   };

   DevSPI_SdCard( DevSPI &a_spi )
     : spi( a_spi ) {};
   bool isInited() const { return inited; }
   uint32_t getCaps() const { return caps; }
   uint8_t  getLastR1() const { return r1; }
   uint16_t getLastR2() const { return r2; }
   uint32_t getLastR7() const { return r7; }
   uint32_t getErrC() const { return err_c; }
   static uint8_t crc7( uint8_t t, uint8_t d );
   static uint8_t crc7( const uint8_t *p, uint16_t l );
   static uint16_t crc16_ccitt( uint16_t crc, uint8_t d );
   static uint16_t crc16( const uint8_t *p, uint16_t l );
   int sd_cmd( uint8_t c, uint32_t arg );
   bool getR1();
   bool getR2();
   bool getR7( uint8_t bad_bits = 0xFE );
   int sd_cmd_r1( uint8_t c, uint32_t arg, uint8_t bad_bits = 0xFE );
   int sd_cmd_r7( uint8_t c, uint32_t arg, uint8_t bad_bits = 0xFE );
   void nec(); // is really need?
   int init();
   int getData( uint8_t *d, int l );
   int putData( const uint8_t *d, int l );


  protected:
   DevSPI &spi;
   bool inited = false;
   uint32_t caps = 0;
   uint32_t sects = 0;
   uint32_t sects_era = 0;
   uint32_t n_try = 1024;
   uint8_t  r1 = 0;
   uint16_t r2 = 0;
   uint32_t r7 = 0;
   uint32_t err_c = 0;
};


#endif

