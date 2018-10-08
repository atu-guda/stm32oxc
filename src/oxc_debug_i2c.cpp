#include <string.h>
#include <stdlib.h>

#include <oxc_devio.h>
#include <oxc_outstream.h>
#include <oxc_debug_i2c.h>


DevI2C *i2c_dbg = nullptr;

static const char i2c_nodev_msg[] = NL "I2C debug device (i2c_dbg) is not set!" NL;
#define CHECK_I2C_DEV \
  if( !i2c_dbg ) { \
    pr( i2c_nodev_msg ); \
    return 2; \
  }


void I2C_print_status( int rc )
{
  if( ! i2c_dbg ) {
    return;
  }
  STDOUT_os;
  os << NL "I2C N: " <<  rc << " state: " << i2c_dbg->getState() << " error: " << i2c_dbg->getErr() << NL;
}

int cmd_i2c_scan( int argc, const char * const * argv )
{
  uint8_t addr_start = (uint8_t)arg2long_d( 1, argc, argv,   2,            0, 127 );
  uint8_t addr_end   = (uint8_t)arg2long_d( 2, argc, argv, 127, addr_start+1, 127 );

  STDOUT_os;
  os << NL "I2C Scan in range [ " << addr_start << " ; "  << addr_end << " ] " NL;
  CHECK_I2C_DEV;

  i2c_dbg->resetDev();

  for( uint8_t addr=addr_start; addr < addr_end; ++addr ) {
    bool rc = i2c_dbg->ping( addr );
    int i_err = i2c_dbg->getErr();
    delay_ms( 10 );
    if( !rc ) {
      continue;
    }
    os << addr << " (0x" <<  HexInt8( addr ) <<  ")  : "  <<  i_err << NL;
  }

  os <<  NL "I2C scan end." NL ;
  return 0;
}
CmdInfo CMDINFO_I2C_SCAN {
  "i2c_scan",  0, cmd_i2c_scan, "[start [end]] - scan I2C in range"
};


int cmd_i2c_send( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;

  uint8_t v = arg2long_d( 1, argc, argv, 0, 0, 255 );
  uint16_t addr = arg2long_d( 2, argc, argv, (uint16_t)(UVAR('p')), 0, 127 );

  STDOUT_os;
  os << NL "I2C Send  " <<  v << " to " << HexInt8( addr ) << NL;

  i2c_dbg->resetDev();

  uint8_t old_addr = i2c_dbg->getAddr();
  i2c_dbg->setAddr( addr );
  int rc = i2c_dbg->send( v );
  i2c_dbg->setAddr( old_addr );
  I2C_print_status( rc );

  return 0;
}
CmdInfo CMDINFO_I2C_SEND {
  "i2c_send",  0, cmd_i2c_send,   "val [addr] - send to I2C (def addr=var[p])"
};

int subcmd_i2c_send_rx( int argc, const char * const * argv, bool is2byte )
{
  CHECK_I2C_DEV;

  STDOUT_os;
  if( argc < 3 ) {
    os <<  "** ERR: reg val required" NL;
    return -1;
  }

  uint8_t reg = arg2long_d( 1, argc, argv, 0, 0, 255 );
  uint8_t v   = arg2long_d( 2, argc, argv, 0, 0, 255 );
  uint8_t addr = (uint8_t)(UVAR('p'));

  os << NL "I2C Send  " <<  v << " = 0x" << HexInt( v ) <<  " to " << HexInt8( addr )
     <<   " : " << HexInt8( reg ) <<  NL;

  i2c_dbg->resetDev();

  uint8_t old_addr = i2c_dbg->getAddr();
  i2c_dbg->setAddr( addr );
  int rc = i2c_dbg->send_reg12( reg, &v, 1, is2byte );
  i2c_dbg->setAddr( old_addr );
  I2C_print_status( rc );

  return 0;
}

int cmd_i2c_send_r1( int argc, const char * const * argv )
{
  return subcmd_i2c_send_rx( argc, argv, false );
}
CmdInfo CMDINFO_I2C_SEND_R1 {
  "i2c_send1", 0, cmd_i2c_send_r1, "reg val - send to I2C(reg), reg_sz=1 (addr=var[p])"
};

int cmd_i2c_send_r2( int argc, const char * const * argv )
{
  return subcmd_i2c_send_rx( argc, argv, true );
}
CmdInfo CMDINFO_I2C_SEND_R2 {
  "i2c_send2",  0,  cmd_i2c_send_r2, "reg val - send to I2C(reg), reg_sz=2 (addr=var[p])"
};


int cmd_i2c_recv( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;

  uint8_t addr  =  (uint8_t)arg2long_d( 1, argc, argv, 0, 0, 127 );
  uint16_t nr   = (uint16_t)arg2long_d( 2, argc, argv, 1, 1, sizeof(gbuf_a) );

  STDOUT_os;
  os <<  NL "I2C Recv from " <<  HexInt8( addr ) << " nr= " <<  nr << NL;

  i2c_dbg->resetDev();

  uint8_t old_addr = i2c_dbg->getAddr();
  i2c_dbg->setAddr( addr );
  int rc = i2c_dbg->recv( (uint8_t*)(&gbuf_a), nr );
  i2c_dbg->setAddr( old_addr );
  I2C_print_status( rc );
  if( rc > 0 ) {
    dump8( gbuf_a, nr );
  }

  return 0;
}
CmdInfo CMDINFO_I2C_RECV {
  "i2c_recv",  0, cmd_i2c_recv,    "[addr [nr]] - recv from I2C (def addr=var[p])"
};

int subcmd_i2c_recv_rx( int argc, const char * const * argv, bool is2byte )
{
  CHECK_I2C_DEV;
  uint8_t addr = (uint8_t)(UVAR('p'));
  uint16_t reg  = (uint16_t)arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );
  uint16_t n =    (uint16_t)arg2long_d( 2, argc, argv, 1, 1, sizeof(gbuf_a) );

  STDOUT_os;
  os <<  NL "I2C recv r2 from  " <<  HexInt8( addr ) <<  " : "  << HexInt16( reg ) <<  " n= " << n;

  i2c_dbg->resetDev();

  uint8_t old_addr = i2c_dbg->getAddr();
  i2c_dbg->setAddr( addr );
  int rc = i2c_dbg->recv_reg12( reg, (uint8_t*)(gbuf_a), n, is2byte );
  i2c_dbg->setAddr( old_addr );
  I2C_print_status( rc );
  if( rc > 0 ) {
    dump8( gbuf_a, n );
  }

  return 0;
}

int cmd_i2c_recv_r1( int argc, const char * const * argv )
{
  return subcmd_i2c_recv_rx( argc, argv, false );
}
CmdInfo CMDINFO_I2C_RECV_R1 {
  "i2c_recv1",  0,  cmd_i2c_recv_r1, "reg [n] - recv from I2C(reg), reg_sz=1 (addr=var[p])"
};


// TODO: combine with r1
int cmd_i2c_recv_r2( int argc, const char * const * argv )
{
  return subcmd_i2c_recv_rx( argc, argv, true );
}
CmdInfo CMDINFO_I2C_RECV_R2 {
  "i2c_recv2",  0,  cmd_i2c_recv_r2, "reg [n] - recv from I2C(reg), reg_sz=2 (addr=var[p])"
};

#undef CHECK_I2C_DEV

