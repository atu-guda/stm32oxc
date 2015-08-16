#ifndef _OXC_SPI_MAX7219_H
#define _OXC_SPI_MAX7219_H


#include <oxc_spi.h>

class DevSPI_MAX7219 {
  public:
   enum { // reg address
     R_NOP = 0x00,    // for cascading
     R_D0  = 0x01,    // Digit 0
     R_D1  = 0x02,    // Digit 1
     R_D2  = 0x03,    // Digit 2
     R_D3  = 0x04,    // Digit 3
     R_D4  = 0x05,    // Digit 4
     R_D5  = 0x06,    // Digit 5
     R_D6  = 0x07,    // Digit 6
     R_D7  = 0x08,    // Digit 7
     R_DEC = 0x09,    // Decode mode per digit: 1 = decode, 0 = segs
     R_INT = 0x0A,    // Intensity 0-0x0F
     R_LIM = 0x0B,    // Scan limit 0-7
     R_SHU = 0x0C,    // Shutdown 0 = down 1 = on
     R_TST = 0x0F     // Test 0 = normal 1 = test
   };

   DevSPI_MAX7219( DevSPI &a_spi )
     : spi( a_spi ) {};

   void wr2( uint8_t ah, uint8_t al ) { uint8_t b[2] = { ah, al }; spi.send( b, 2 ); }
   void test_on()  { wr2( R_TST, 0x01 ); }
   void test_off() { wr2( R_TST, 0x00 ); }
   void on()       { wr2( R_SHU, 0x01 ); }
   void off()      { wr2( R_SHU, 0x00 ); }
   void setDecode( uint8_t poss ) { wr2( R_DEC, poss ); }
   void setLimit(  uint8_t mpos ) { wr2( R_LIM, mpos ); }
   void setIntens( uint8_t v )    { wr2( R_INT, v ); }
   void setDigit( uint8_t pos, uint8_t v )  { wr2( R_D0+pos, v ); }
   void setXDigit( uint8_t pos, uint8_t v ); // bitmap mode required
   void setDigits( const uint8_t *vs, uint8_t dpos = 0xFF, uint8_t st=0, uint8_t en=7 );
   void setXDigits( const uint8_t *vs, uint8_t dpos = 0xFF, uint8_t st=0, uint8_t en=7 );
   void setVal( int v, uint8_t dpos = 0xFF, uint8_t st=0, uint8_t l=7 );
   void setUVal( unsigned v, uint8_t dpos = 0xFF, uint8_t st=0, uint8_t l=8 );
   void setXVal( unsigned v, uint8_t dpos = 0xFF, uint8_t st=0, uint8_t l=8 );
   void setSameVal( uint8_t st=0, uint8_t l=8, uint8_t = 0 ); // fill
   void clsDig( uint8_t st=0, uint8_t l=8 ) { setSameVal( st, l, 0x0F ); }
   void clsBit( uint8_t st=0, uint8_t l=8 ) { setSameVal( st, l, 0x00 ); }

  protected:
   DevSPI &spi;
};

// if decode in ON: digits 0-9, A=-, B=E, C=H, D=L, E=P, F=' ' + 0x80='.'
// OFF:
//    40
// 02    20
//    01
// 04    10
//    08    80

#endif

