#include <cstring>

#include <oxc_auto.h>
#include <oxc_main.h>

#include <oxc_spimem_at.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to SPI based flash memory" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test common"  );
DCL_CMD_REG( spimem_read, 'R', " [ n [ offset ]] - read from mem"  );
DCL_CMD_REG( spimem_write, 'W', " [ n [ offset [c0 [dc]]]] - write"  );
DCL_CMD_REG( spimem_erase,  0, "- ERASE CHIP!"  );
DCL_CMD_REG( spimem_sector0_erase,  'Z', "- erase sector 0"  );




PinOut nss_pin( BOARD_SPI_DEFAULT_PIN_SNSS );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );
DevSPIMem_AT memspi( spi_d );

int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 1000;
  UVAR_n = 20;
  UVAR_r = 0x20; // default bytes to read

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_8 ) != HAL_OK ) {
    die4led( 0x04_mask );
  }

  spi_d.initSPI();

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


#define DLY_T delay_mcs( 2 );

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int nd     = imin( UVAR_r, sizeof(gbuf_b) );
  std_out <<  NL "# Test0: nd= "  <<  nd  <<  NL;

  if( UVAR_d > 0 ) { // debug: for logic analizer start
    nss_pin.write( 0 );
    DLY_T;
    nss_pin.write( 1 );
    DLY_T;
  }

  uint16_t chip_id = memspi.read_id();
  pr_shx( chip_id );

  std_out << "# status= " << memspi.status()  <<  NL;

  memset( gbuf_b, '\x00', sizeof( gbuf_b ) );

  int rc = memspi.read( (uint8_t*)gbuf_b, 0x00, nd );
  std_out <<  "#  Read Before: rc = "  <<  rc  <<  NL;
  dump8( gbuf_b, nd );
  std_out << "# status= " << memspi.status()  <<  NL;

  for( int i=0; i<nd; ++i ) {
    gbuf_a[i] = (char)( '0' + i );
  }

  rc = memspi.write( (uint8_t*)gbuf_a, 0x00, nd );
  std_out <<  NL "# Write: rc= "  <<  rc  <<  NL;
  std_out << "# status= " << memspi.status()  <<  NL;

  rc = memspi.read( (uint8_t*)gbuf_b, 0x00, nd );
  std_out <<  "# Read After: rc = "  <<  rc  <<  NL;
  dump8( gbuf_b, nd );
  std_out << "# status= " << memspi.status()  <<  NL;

  rc = memspi.read( (uint8_t*)gbuf_b, 0x03, nd );
  std_out <<  "# Read After with offset 3: rc = "  <<  rc  <<  NL;
  dump8( gbuf_b, nd );
  std_out << "# status= " << memspi.status()  <<  NL;

  return 0;
}

int cmd_spimem_write( int argc, const char * const * argv )
{
  unsigned n       = arg2long_d( 1, argc, argv, 0x20, 1, sizeof(gbuf_b) );
  unsigned ofs     = arg2long_d( 2, argc, argv,    0, 0, 0x00FFFFFF  );
  uint8_t  c0      = arg2long_d( 3, argc, argv,  '@', 0, 0xFF  );
  uint8_t  dc      = arg2long_d( 4, argc, argv,    1, 0, 0xFF  );

  std_out <<  NL "# Write: n= "  <<  n << " ofs= " << ofs  << " ( 0x" << HexInt( ofs )
          << " ) c0=" << HexInt8( c0 ) << " dc= " << dc << NL;

  memset( gbuf_b, '\x00', sizeof( gbuf_b ) );
  uint8_t c = c0;
  for( decltype(+n) i=0; i<n; ++i ) {
    gbuf_b[i] = c;
    c += dc;
  }

  auto rc = memspi.write( (uint8_t*)gbuf_b, ofs, n );
  std_out << "# rc= " << rc << " status= " << memspi.status()  <<  NL;

  return 0;
}

int cmd_spimem_read( int argc, const char * const * argv )
{
  unsigned n       = arg2long_d( 1, argc, argv, 0x20, 1, sizeof(gbuf_b) );
  unsigned ofs     = arg2long_d( 2, argc, argv,    0, 0, 0x00FFFFFF  );

  std_out <<  NL "# Read: n= "  <<  n << " ofs= " << ofs  << " ( 0x" << HexInt( ofs ) << " )" NL;

  memset( gbuf_b, '\x00', sizeof( gbuf_b ) );

  auto rc = memspi.read( (uint8_t*)gbuf_b, ofs, n );
  dump8( gbuf_b, n );
  std_out << "# rc= " << rc << " status= " << memspi.status()  <<  NL;

  return 0;
}

int cmd_spimem_erase( int argc, const char * const * argv )
{
  int rc = memspi.erase_chip();

  return rc;
}

int cmd_spimem_sector0_erase( int argc, const char * const * argv )
{
  int rc = memspi.erase_sector( 0x000000 );

  return rc;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

