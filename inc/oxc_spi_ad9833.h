#ifndef _OXC_SPI_AD9833_H
#define _OXC_SPI_AD9833_H

#include <oxc_spi.h>

// interface to AD9833 - programmable waveform generator (DSS)

class DevAD9833 {
  public:
  enum CmdBits { // both bytes
    mode_sin      = 0x00, // [1]
    mode_trangle  = 0x01,
    div2          = 0x08, // *2 meandr, if  set         =  = sin freq
    opbiten       = 0x20,
    sleep12       = 0x40, // DAC off, only meandr possible
    sleep1        = 0x80, // clock sleep, DAC active
    // MSB:
    rst           = 0x01,
    pselect       = 0x04,
    fselect       = 0x08,
    hlb           = 0x10, //
    b28           = 0x20,
    // addrs
    freq1_addr    = 0x40,
    freq2_addr    = 0x80,
    freq_valmask  = 0x3F, // only MSB
    phase1_addr   = 0xC0,
    phase2_addr   = 0xE0,
    phase_valmask = 0x0F, // only MSB
  };
   DevAD9833( DevSPI &a_spi, uint32_t a_freq_in = 25000000 )
     : spi( a_spi ), freq_in( a_freq_in ) {};
   bool sendCurrMode() { return( 2 ==  spi.send( cmd_mode, 2 ) ); }
   bool addToMode0( uint8_t bits ) { cmd_mode[0] |= bits;  return sendCurrMode(); }
   bool addToMode1( uint8_t bits ) { cmd_mode[1] |= bits;  return sendCurrMode(); }
   bool subFromMode0( uint8_t bits ) { cmd_mode[0] &= ~bits;  return sendCurrMode(); }
   bool subFromMode1( uint8_t bits ) { cmd_mode[1] &= ~bits;  return sendCurrMode(); }
   bool resetOut() { return addToMode0( rst ); };
   bool setMode( uint8_t cmd_msb, uint8_t cmd_lsb );
   bool initFreq( int32_t freq, uint8_t cmd2 = 0, bool isFreq2 = false );
   bool setFreq( int32_t freq, bool isFreq2  = false );
   bool setPhase( int16_t phase, bool isPhase2  = false ); // 0-4096
   bool switchToFreq( bool isFreq2 );
   bool switchToFreq1() { return subFromMode0( fselect ); }
   bool switchToFreq2() { return addToMode0( fselect ); }
   bool switchToPhase( bool isPhase2 );
   bool switchToPhase1() { return subFromMode0( pselect ); }
   bool switchToPhase2() { return addToMode0( pselect ); }
  protected:
   DevSPI &spi;
   uint32_t freq_in;
   uint8_t cmd_mode[2] = { 0x20, 0x00 };
};



#include <oxc_spi.h>


#endif

