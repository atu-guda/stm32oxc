#include <oxc_auto.h>
#include <oxc_spi_debug.h>

CmdInfo CMDINFO_S1RN { "s1rn", '\0', cmd_send1recvN_spi, " send_byte n_recv - send 1 recv N"  };
CmdInfo CMDINFO_SENDR { "sendr", '\0', cmd_sendr_spi, "[0xXX ...] - send bytes, recv UVAR('r')"  };
CmdInfo CMDINFO_DUPLEX { "duplex", '\0', cmd_duplex_spi, "[0xXX ...] - send/recv bytes"  };
CmdInfo CMDINFO_RECV { "recv", '\0', cmd_recv_spi, "[N] recv bytes"  };
CmdInfo CMDINFO_RESETSPI { "reset_spi", '\0', cmd_reset_spi, " - reset spi"  };
CmdInfo CMDINFO_SENDLOOPSPI { "sendloop_spi", '\0', cmd_sendloop_spi, " N [val0 [val1]] - send N vals via SPI"  };




int cmd_send1recvN_spi( int argc, const char * const * argv )
{
  uint8_t sv = arg2long_d( 1, argc, argv, 0x15, 0, 0xFF );
  int nd     = arg2long_d( 2, argc, argv,    2, 0, sizeof(gbuf_a) );
  std_out << NL "# Test0: sv= "  << HexInt8( sv ) << " nd= "  <<  nd  <<  NL;

  // spi_d.resetDev();

  dbg_pin.set(); delay_bad_100ns( UVAR('q') );
  int rc = spi_d.send_recv( sv, (uint8_t*)gbuf_a, nd );
  dbg_pin.reset();
  // int rc = spi_d.send( (uint8_t)sv );
  // int rc = spi_d.recv( (uint8_t*)gbuf_a, imin(UVAR('r'),sizeof(gbuf_a)) );


  std_out << "# rc = " << rc << NL;
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  }
  spi_d.pr_info();

  return 0;
}

int cmd_sendr_spi( int argc, const char * const * argv )
{
  uint8_t sbuf[MAX_ARGS+1];
  uint16_t ns = argc - 1;

  for( uint16_t i = 0; i<ns; ++i ) {
    uint8_t t = arg2long_d( i+1, argc, argv, 0, 0, 0xFF );
    sbuf[i] = t;
  }

  int nd = imin( UVAR('r'), sizeof(gbuf_a) );
  std_out <<  NL "# Send/recv: ns= "  <<  ns  <<  " nd= "  <<  nd  <<  "* to send: " NL;
  dump8( sbuf, ns );

  dbg_pin.set(); delay_bad_100ns( UVAR('q') );
  int rc = spi_d.send_recv( sbuf, ns, (uint8_t*)gbuf_a, nd );
  dbg_pin.reset();

  std_out << "# rc= " << rc << NL;
  if( rc > 0 || nd == 0 ) {
    std_out <<  "# recv: " NL ;
    dump8( gbuf_a, rc );
  } else {
    std_out <<  "#** Error, code= "  << spi_d.getErr() <<  NL;
  }
  delay_ms( 10 );

  spi_d.pr_info();

  return 0;
}

int cmd_recv_spi( int argc, const char * const * argv )
{
  int nd = arg2long_d( 1, argc, argv, UVAR('r'), 1, sizeof(gbuf_a) );

  std_out <<  NL "# Recv: nd= "  <<  nd  <<  NL;

  dbg_pin.set(); delay_bad_100ns( UVAR('q') );
  int rc = spi_d.recv( (uint8_t*)gbuf_a, nd );
  dbg_pin.reset();

  pr_sdx( rc );
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  } else {
    std_out <<  "#** Error, code= "  << spi_d.getErr()<<  NL;
  }
  delay_ms( 10 );

  spi_d.pr_info();

  return 0;
}

int cmd_duplex_spi( int argc, const char * const * argv )
{
  uint8_t sbuf[MAX_ARGS+1];
  uint16_t ns = argc - 1;

  for( uint16_t i = 0; i<ns; ++i ) {
    uint8_t t = arg2long_d( i+1, argc, argv, 0, 0, 0xFF );
    sbuf[i] = t;
  }

  std_out <<  NL "# Duplex: ns= "  <<  ns  <<  NL;
  dump8( sbuf, ns );

  dbg_pin.set(); delay_bad_100ns( UVAR('q') );
  int rc = spi_d.duplex( sbuf, (uint8_t*)gbuf_a, ns );
  dbg_pin.reset();

  std_out << "# rc = " << rc << NL;
  if( rc > 0 ) {
    dump8( gbuf_a, rc );
  } else {
    std_out <<  "#** Error, code= "  << spi_d.getErr() <<  NL;
  }
  delay_ms( 10 );

  spi_d.pr_info();

  return 0;
}

int cmd_sendloop_spi( int argc, const char * const * argv )
{
  int n       = arg2long_d( 1, argc, argv,    1, 1, 10000000 );
  uint8_t sv0 = arg2long_d( 2, argc, argv, 0x55, 0, 0xFF );
  uint8_t sv1 = arg2long_d( 3, argc, argv,  sv0, 0, 0xFF );
  std_out << NL "# sendloop_spi: sv0= "  << HexInt8( sv0 ) << " sv1= " << HexInt8( sv1 )
          << " n= "  <<  n  <<  NL;

  dbg_pin.set(); delay_bad_100ns( UVAR('q') );
  for( int i=0; i<n && ! break_flag; ++i ) {
    spi_d.send( (i&1) ? sv1 : sv0 );
  }
  dbg_pin.reset();

  return 0;
}


int cmd_reset_spi( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  spi_d.resetDev();

  spi_d.pr_info();

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

