#include <string.h>
#include <stdlib.h>

#include <oxc_debug_i2c.h>


DevI2C *i2c_dbg = nullptr;

static const char i2c_nodev_msg[] = NL "I2C debug device (i2c_dbg) is not set!" NL;
#define CHECK_I2C_DEV \
  if( !i2c_dbg ) { \
    pr( i2c_nodev_msg ); \
    return 2; \
  }
#define I2C_PRINT_STATUS  pr( NL "I2C N: " ); pr_d( rc ); \
  pr( " state: " ); pr_d( i2c_dbg->getState() ); \
  pr( " error: " ); pr_d( i2c_dbg->getErr() ); \
  pr( NL );

int cmd_i2c_scan( int argc, const char * const * argv )
{
  uint8_t addr_start = (uint8_t)arg2long_d( 1, argc, argv,   2,            0, 127 );
  uint8_t addr_end   = (uint8_t)arg2long_d( 2, argc, argv, 127, addr_start+1, 127 );

  pr( NL "I2C Scan in range [ " ); pr_d( addr_start );
  pr( " ; " ); pr_d( addr_end ); pr( " ] " NL );
  CHECK_I2C_DEV;

  i2c_dbg->resetDev();

  for( uint8_t addr=addr_start; addr < addr_end; ++addr ) {
    bool rc = i2c_dbg->ping( addr );
    int i_err = i2c_dbg->getErr();
    delay_ms( 10 );
    if( !rc ) {
      continue;
    }
    pr_d( addr ); pr( " (0x" ); pr_h( addr ); pr( ") " );
    pr(" : " );      pr_d( i_err ); pr( NL );
  }

  pr( NL "I2C scan end." NL );
  return 0;
}
CmdInfo CMDINFO_I2C_SCAN {
  "scan",  'C', cmd_i2c_scan, "[start [end]] - scan I2C in range"
};


int cmd_i2c_send( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;

  uint8_t v = arg2long_d( 1, argc, argv, 0, 0, 255 );
  uint16_t addr = arg2long_d( 2, argc, argv, (uint16_t)(UVAR('p')), 0, 127 );

  pr( NL "I2C Send  " ); pr_d( v );
  pr( " to " ); pr_h( addr ); pr( NL );

  i2c_dbg->resetDev();

  uint8_t old_addr = i2c_dbg->getAddr();
  i2c_dbg->setAddr( addr );
  int rc = i2c_dbg->send( v );
  i2c_dbg->setAddr( old_addr );
  I2C_PRINT_STATUS;

  return 0;
}
CmdInfo CMDINFO_I2C_SEND {
  "send",  'S', cmd_i2c_send,   "val [addr] - send to I2C (def addr=var[p])"
};

int subcmd_i2c_send_rx( int argc, const char * const * argv, bool is2byte )
{
  CHECK_I2C_DEV;
  if( argc < 3 ) {
    pr( "** ERR: reg val required" NL );
    return -1;
  }

  uint8_t reg = arg2long_d( 1, argc, argv, 0, 0, 255 );
  uint8_t v   = arg2long_d( 2, argc, argv, 0, 0, 255 );
  uint8_t addr = (uint8_t)(UVAR('p'));

  pr( NL "I2C Send  " ); pr_d( v ); pr( " = 0x" ); pr_h( v );
  pr( " to " ); pr_h( addr ); pr( ": " ); pr_h( reg ); pr( NL );

  i2c_dbg->resetDev();

  uint8_t old_addr = i2c_dbg->getAddr();
  i2c_dbg->setAddr( addr );
  int rc = i2c_dbg->send_reg12( reg, &v, 1, is2byte );
  i2c_dbg->setAddr( old_addr );
  I2C_PRINT_STATUS;

  return 0;
}

int cmd_i2c_send_r1( int argc, const char * const * argv )
{
  return subcmd_i2c_send_rx( argc, argv, false );
}
CmdInfo CMDINFO_I2C_SEND_R1 {
  "send1", 0, cmd_i2c_send_r1, "reg val - send to I2C(reg), reg_sz=1 (addr=var[p])"
};

int cmd_i2c_send_r2( int argc, const char * const * argv )
{
  return subcmd_i2c_send_rx( argc, argv, true );
}
CmdInfo CMDINFO_I2C_SEND_R2 {
  "send2",  0,  cmd_i2c_send_r2, "reg val - send to I2C(reg), reg_sz=2 (addr=var[p])"
};


int cmd_i2c_recv( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;

  uint8_t addr  =  (uint8_t)arg2long_d( 1, argc, argv, 0, 0, 127 );
  uint16_t nr   = (uint16_t)arg2long_d( 2, argc, argv, 1, 1, sizeof(gbuf_a) );

  pr( NL "I2C Recv from " );  pr_h( addr ); pr( " nr= " ); pr_d( nr ); pr( NL );

  i2c_dbg->resetDev();

  uint8_t old_addr = i2c_dbg->getAddr();
  i2c_dbg->setAddr( addr );
  int rc = i2c_dbg->recv( (uint8_t*)(&gbuf_a), nr );
  i2c_dbg->setAddr( old_addr );
  I2C_PRINT_STATUS;
  if( rc > 0 ) {
    dump8( gbuf_a, nr );
  }

  return 0;
}
CmdInfo CMDINFO_I2C_RECV {
  "recv",  'R', cmd_i2c_recv,    "[addr [nr]] - recv from I2C (def addr=var[p])"
};

int subcmd_i2c_recv_rx( int argc, const char * const * argv, bool is2byte )
{
  CHECK_I2C_DEV;
  uint8_t addr = (uint8_t)(UVAR('p'));
  uint16_t reg  = (uint16_t)arg2long_d( 1, argc, argv, 0, 0, 0xFFFF );
  uint16_t n =    (uint16_t)arg2long_d( 2, argc, argv, 1, 1, sizeof(gbuf_a) );

  pr( NL "I2C recv r2 from  " );
  pr_h( addr ); pr( ":" ); pr_h( reg ); pr( " n= " ); pr_d( n );

  i2c_dbg->resetDev();

  uint8_t old_addr = i2c_dbg->getAddr();
  i2c_dbg->setAddr( addr );
  int rc = i2c_dbg->recv_reg12( reg, (uint8_t*)(gbuf_a), n, is2byte );
  i2c_dbg->setAddr( old_addr );
  I2C_PRINT_STATUS;
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
  "recv1",  0,  cmd_i2c_recv_r1, "reg [n] - recv from I2C(reg), reg_sz=1 (addr=var[p])"
};


// TODO: combine with r1
int cmd_i2c_recv_r2( int argc, const char * const * argv )
{
  return subcmd_i2c_recv_rx( argc, argv, true );
}
CmdInfo CMDINFO_I2C_RECV_R2 {
  "recv2",  0,  cmd_i2c_recv_r2, "reg [n] - recv from I2C(reg), reg_sz=2 (addr=var[p])"
};

#undef CHECK_I2C_DEV
#undef I2C_PRINT_STATUS

