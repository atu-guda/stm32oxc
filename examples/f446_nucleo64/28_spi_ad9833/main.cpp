#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

// TODO: move to library files

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

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_sendr_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_SENDR { "sendr", 'S', cmd_sendr_spi, "[0xXX ...] - send bytes, recv UVAR('r')"  };

int cmd_duplex_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_DUPLEX { "duplex", 'U', cmd_duplex_spi, "[0xXX ...] - send/recv bytes"  };

int cmd_recv_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_RECV { "recv", 'R', cmd_recv_spi, "[N] recv bytes"  };

int cmd_reset_spi( int argc, const char * const * argv );
CmdInfo CMDINFO_RESETSPI { "reset_spi", 'Z', cmd_reset_spi, " - reset spi"  };

  const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SENDR,
  &CMDINFO_RECV,
  &CMDINFO_DUPLEX,
  &CMDINFO_RESETSPI,
  nullptr
};



PinsOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS, 1 ); //  to test GPIO
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );

DevAD9833 gener( spi_d );

int SPI_init_clockhigh( uint32_t baud_presc )
{
  spi_h.Instance               = BOARD_SPI_DEFAULT;
  spi_h.Init.Mode              = SPI_MODE_MASTER;
  spi_h.Init.Direction         = SPI_DIRECTION_2LINES;
  spi_h.Init.DataSize          = SPI_DATASIZE_8BIT;
  spi_h.Init.CLKPolarity       = SPI_POLARITY_HIGH;
  spi_h.Init.CLKPhase          = SPI_PHASE_1EDGE;
  spi_h.Init.NSS               = SPI_NSS_SOFT;
  spi_h.Init.BaudRatePrescaler = baud_presc; // SPI_BAUDRATEPRESCALER_2; ... _256
  spi_h.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  spi_h.Init.TIMode            = SPI_TIMODE_DISABLED;
  spi_h.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  return HAL_SPI_Init( &spi_h );
}

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;
  UVAR('r') = 0x0; // default bytes to read

  if( SPI_init_clockhigh( SPI_BAUDRATEPRESCALER_256 ) != HAL_OK ) {
    die4led( 0x04 );
  }
  // nss_pin.initHW();
  //nss_pin.set(1);
  spi_d.setMaxWait( 500 );
  spi_d.initSPI();


  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  default_main_loop();
  vTaskDelete(NULL);
}

#define DLY_T delay_mcs( 10 );


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t freq = arg2long_d( 1, argc, argv, 1000, 0, 12500000 );
  uint8_t  cmd2 = arg2long_d( 2, argc, argv, 0x00, 0, 255 );
  uint32_t div0 = (uint32_t)( ((uint64_t)freq <<  28) / 25000000 );
  pr( NL "Test0: freq= " ); pr_d( freq ); pr( "  div0= " ); pr_d( div0 ); pr( " = " ); pr_h( div0 );
  pr( NL );

  gener.initFreq( freq, cmd2 );
  gener.setFreq( freq/2, true );

  for( int i=0; i<10; ++i ) {
    delay_ms( 1000 );
    gener.switchToFreq( i & 1 );
  }

  gener.addToMode1( DevAD9833::sleep1 );
  delay_ms( 2000 );
  gener.subFromMode1( DevAD9833::sleep1 );
  delay_ms( 1000 );
  gener.addToMode1( DevAD9833::sleep12 );
  delay_ms( 2000 );
  gener.subFromMode1( DevAD9833::sleep12 );
  delay_ms( 1000 );


  return 0;
}

int cmd_sendr_spi( int argc, const char * const * argv )
{
  uint8_t sbuf[16]; // really used not more then 9 - max args
  uint16_t ns = argc - 1;

  for( uint16_t i = 0; i<ns; ++i ) {
    uint8_t t = arg2long_d( i+1, argc, argv, 0, 0, 0xFF );
    sbuf[i] = t;
  }

  int nd = imin( UVAR('r'), sizeof(gbuf_a) );
  pr( NL "Send/recv: ns= " ); pr_d( ns ); pr( " nd= " ); pr_d( nd );
  pr( "* to send: " NL );
  dump8( sbuf, ns );

  int rc = spi_d.send_recv( sbuf, ns, (uint8_t*)gbuf_a, nd );

  pr_sdx( rc );
  if( rc > 0 ) {
    pr( "* recv: " NL );
    dump8( gbuf_a, rc );
  } else {
    pr( "** Error, code= " ); pr_d( spi_d.getErr() ); pr( NL );
  }
  delay_ms( 10 );

  spi_d.pr_info();

  return 0;
}

int cmd_recv_spi( int argc, const char * const * argv )
{
  int nd = arg2long_d( 1, argc, argv, UVAR('r'), 1, sizeof(gbuf_a) );

  pr( NL "Recv: nd= " ); pr_d( nd );
  pr( NL );

  int rc = spi_d.recv( (uint8_t*)gbuf_a, nd );

  pr_sdx( rc );
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  } else {
    pr( "** Error, code= " ); pr_d( spi_d.getErr() ); pr( NL );
  }
  delay_ms( 10 );

  spi_d.pr_info();

  return 0;
}

int cmd_duplex_spi( int argc, const char * const * argv )
{
  uint8_t sbuf[16]; // really used not more then 9 - max args
  uint16_t ns = argc - 1;

  for( uint16_t i = 0; i<ns; ++i ) {
    uint8_t t = arg2long_d( i+1, argc, argv, 0, 0, 0xFF );
    sbuf[i] = t;
  }

  pr( NL "Duplex: ns= " ); pr_d( ns );
  pr( NL );
  dump8( sbuf, ns );

  int rc = spi_d.duplex( sbuf, (uint8_t*)gbuf_a, ns );

  pr_sdx( rc );
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  } else {
    pr( "** Error, code= " ); pr_d( spi_d.getErr() ); pr( NL );
  }
  delay_ms( 10 );

  spi_d.pr_info();

  return 0;
}


int cmd_reset_spi( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  spi_d.resetDev();

  spi_d.pr_info();

  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

