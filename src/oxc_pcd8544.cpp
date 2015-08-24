#include <oxc_pcd8544.h>

using namespace std;


int  PCD8544::cmd( const uint8_t *cmds, uint16_t n )
{
  rst_dc.reset( BIT_DC );
  int rc = spi_d.send( cmds, n );
  return rc;
}

int PCD8544::xcmd( uint8_t acmd )
{
  uint8_t cmds[] = { FUNC_XCMD, acmd, FUNC_NCMD };
  return cmd( cmds, 3 );
}

int PCD8544::ncmd( uint8_t acmd )
{
  uint8_t cmds[] = { FUNC_NCMD, acmd };
  return cmd( cmds, 2 );
}

int PCD8544::data( const uint8_t *d, uint16_t n )
{
  rst_dc.set( BIT_DC );
  int rc = spi_d.send( d, n );
  rst_dc.reset( BIT_DC );
  return rc;
}



int  PCD8544::init()
{
  rst_dc.initHW();
  spi_d.initSPI();
  rst_dc.set( BIT_RST );
  rst_dc.reset( BIT_DC );
  delay_ms( 1 );
  reset();
  static const uint8_t on_cmd[] = { FUNC_XCMD, XCMD_VOP_DFLT, XCMD_TCX_DFLT, XCMD_BIAS_DFLT,
    FUNC_NCMD, CMD_CFG_DFLT, CMD_Y_POS, CMD_X_POS
  };
  return spi_d.send( on_cmd, sizeof(on_cmd) );
}



int  PCD8544::out( PixBuf1V &pb )
{
  static const uint8_t goto_00[] = { FUNC_NCMD, CMD_Y_POS, CMD_X_POS };
  cmd( goto_00, sizeof(goto_00) );
  uint8_t *buf = pb.fb();
  return data( buf, MEM_SZ );
}

