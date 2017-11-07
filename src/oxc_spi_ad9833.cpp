#include <oxc_spi_ad9833.h>

bool DevAD9833::setMode( uint8_t cmd_msb, uint8_t cmd_lsb )
{
  cmd_mode[0] = cmd_msb; cmd_mode[1] = cmd_lsb;
  return sendCurrMode();
}

bool DevAD9833::initFreq( int32_t freq, uint8_t cmd2, bool isFreq2 )
{
  uint32_t div0 = (uint32_t)( ((uint64_t)freq <<  28) / 25000000 );

  uint8_t buf[10];
  uint8_t freq_prefix = isFreq2 ? freq2_addr : freq1_addr;
  buf[0] = b28 | rst; buf[1]  = 0x00; // rst
  buf[2] = freq_prefix | (uint8_t)( (div0 >> 8)  & freq_valmask ); // LSB
  buf[3] = uint8_t( div0 );
  buf[4] = freq_prefix | (uint8_t)( (div0 >> 22) & freq_valmask ); // MSB
  buf[5] = uint8_t( div0 >> 14 );
  buf[6] = 0xC0; buf[7]  = 0x00; // phase_0
  buf[8] = isFreq2 ? ( b28 | pselect ) : ( b28 ) ; buf[9]  = cmd2; // !rst
  // dump8( buf, sizeof(buf) );
  cmd_mode[0] = buf[8]; cmd_mode[1] = buf[9];
  return( sizeof(buf) ==  spi.send( buf, sizeof(buf) ) );
}

bool DevAD9833::setFreq( int32_t freq, bool isFreq2 )
{
  uint32_t div0 = (uint32_t)( ((uint64_t)freq <<  28) / 25000000 );

  uint8_t buf[4];
  uint8_t freq_prefix = isFreq2 ? freq2_addr : freq1_addr;
  buf[0] = freq_prefix | (uint8_t)( (div0 >> 8)  & freq_valmask ); // LSB
  buf[1] = uint8_t( div0 );
  buf[2] = freq_prefix | (uint8_t)( (div0 >> 22) & freq_valmask ); // MSB
  buf[3] = uint8_t( div0 >> 14 );
  return( sizeof(buf) ==  spi.send( buf, sizeof(buf) ) );
}

bool DevAD9833::setPhase( int16_t phase, bool isPhase2 )
{
  uint8_t buf[2];
  uint8_t phase_prefix = isPhase2 ? phase2_addr : phase1_addr;
  buf[0] = phase_prefix | (uint8_t)( ( phase >> 8)  & freq_valmask );
  buf[1] = uint8_t( phase );
  return( sizeof(buf) ==  spi.send( buf, sizeof(buf) ) );
}

bool DevAD9833::switchToFreq( bool isFreq2 )
{
  if( isFreq2 ) {
    return addToMode0( fselect );
  }
  return subFromMode0( fselect );
}


bool DevAD9833::switchToPhase( bool isPhase2 )
{
  if( isPhase2 ) {
    return addToMode0( pselect );
  }
  return subFromMode0( pselect );
}
