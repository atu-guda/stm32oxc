#include <string.h>
#include <stdlib.h>

#include <oxc_debug_i2c.h>


I2C_HandleTypeDef *i2ch_dbg = nullptr;

// timeout
#define I2C_TO 100
#define RESET_I2C  __HAL_I2C_DISABLE( i2ch_dbg ); delay_ms( 10 ); __HAL_I2C_ENABLE( i2ch_dbg );  delay_ms( 10 );

static const char i2c_nodev_msg[] = NL "I2C debug device (i2ch_dbg) is not set!" NL;
#define CHECK_I2C_DEV \
  if( !i2ch_dbg ) { \
    pr( i2c_nodev_msg ); \
    return 2; \
  }
#define I2C_PRINT_STATUS  pr( NL "I2C action status  " ); pr_d( rc ); pr( NL );

int cmd_i2c_scan( int argc, const char * const * argv )
{
  uint8_t addr_start = (uint8_t)arg2long_d( 1, argc, argv,   2,            0, 127 );
  uint8_t addr_end   = (uint8_t)arg2long_d( 2, argc, argv, 127, addr_start+1, 127 );

  pr( NL "I2C Scan in range [ " ); pr_d( addr_start );
  pr( " ; " ); pr_d( addr_end ); pr( " ] " NL );
  CHECK_I2C_DEV;

  RESET_I2C;

  for( uint8_t addr=addr_start; addr < addr_end; ++addr ) {
    uint16_t ad2 = addr << 1;
    // init_i2c();
    HAL_StatusTypeDef rc = HAL_I2C_IsDeviceReady( i2ch_dbg, ad2, 3, I2C_TO );
    int i_err = i2ch_dbg->ErrorCode;
    delay_ms( 10 );
    if( rc != HAL_OK ) {
      continue;
    }
    pr_d( addr ); pr( " (0x" ); pr_h( addr ); pr( ") " );
    pr(" : " );      pr_d( i_err ); pr( NL );
  }

  // ev  = i2c_dbg.getEv();
  // pr_shx( ev );

  pr( NL "I2C scan end." NL );

  return 0;
}
CmdInfo CMDINFO_I2C_SCAN {
  "scan",  'C', cmd_i2c_scan, "[start [end]] - scan I2C in range"
};


int cmd_i2c_send( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;
  if( argc < 2 ) {
    pr( "error: value required" NL );
    return 1;
  }
  uint8_t v = 0;
  long l;
  char *eptr;

  l = strtol( argv[1], &eptr, 0 );
  if( eptr != argv[1] ) {
    v = (uint8_t)(l);
  }

  uint16_t addr = (uint16_t)(user_vars['p'-'a']);

  if( argc > 2 ) {
    l = strtol( argv[2], &eptr, 0 );
    if( eptr != argv[2] ) {
      addr = (uint16_t)(l);
    }
  }
  uint16_t addr2 = addr<<1;

  pr( NL "I2C Send  " ); pr_d( v );
  pr( " to " ); pr_h( addr ); pr( NL );

  RESET_I2C;

  HAL_StatusTypeDef rc = HAL_I2C_Master_Transmit( i2ch_dbg, addr2, &v, 1, I2C_TO );
  I2C_PRINT_STATUS;

  // i2c_print_status( i2c_dbg );

  return 0;
}
CmdInfo CMDINFO_I2C_SEND {
  "send",  'S', cmd_i2c_send,   "val [addr] - send to I2C (def addr=var[p])"
};


int cmd_i2c_send_r1( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;
  if( argc < 3 ) {
    pr( "** ERR: reg val required" NL );
    return -1;
  }
  uint16_t addr = (uint16_t)(user_vars['p'-'a']);
  uint16_t addr2 = addr<<1;
  uint8_t reg = (uint8_t) strtol( argv[1], 0, 0 );
  uint8_t val = (uint8_t) strtol( argv[2], 0, 0 );

  pr( NL "I2C Send  " ); pr_d( val );
  pr( " to " ); pr_h( addr ); pr( ": " ); pr_h( reg ); pr( NL );

  RESET_I2C;

  HAL_StatusTypeDef rc = HAL_I2C_Mem_Write( i2ch_dbg, addr2, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, I2C_TO );
  I2C_PRINT_STATUS;

  // i2c_print_status( i2c_dbg );

  return 0;
}
CmdInfo CMDINFO_I2C_SEND_R1 {
  "send1", 0, cmd_i2c_send_r1, "reg val - send to I2C(reg), reg_sz=1 (addr=var[p])"
};

int cmd_i2c_send_r2( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;
  if( argc < 3 ) {
    pr( "** ERR: reg val required" NL );
    return -1;
  }
  uint8_t addr = (uint8_t)(user_vars['p'-'a']);
  uint16_t addr2 = addr<<1;
  uint16_t reg = (uint16_t) strtol( argv[1], 0, 0 );
  uint8_t  val = (uint8_t) strtol( argv[2], 0, 0 );

  pr( NL "I2C Send  r2: " ); pr_d( val );
  pr( " to " ); pr_h( addr ); pr( ": " ); pr_h( reg ); pr( NL );

  RESET_I2C;

  HAL_StatusTypeDef rc = HAL_I2C_Mem_Write( i2ch_dbg, addr2, reg, I2C_MEMADD_SIZE_16BIT, &val, 1, I2C_TO );
  I2C_PRINT_STATUS;

  // i2c_print_status( i2c_dbg );

  return 0;
}
CmdInfo CMDINFO_I2C_SEND_R2 {
  "send2",  0,  cmd_i2c_send_r2, "reg val - send to I2C(reg), reg_sz=2 (addr=var[p])"
};


int cmd_i2c_recv( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;
  // uint8_t v = 0;
  uint8_t  addr = (uint8_t)(user_vars['p'-'a']);
  long l;
  int nr = 1;
  char *eptr;

  if( argc > 1 ) {
    l = strtol( argv[1], &eptr, 0 );
    if( eptr != argv[1] ) {
      addr = (uint8_t)(l);
    }
  }

  if( argc > 2 ) {
    l = strtol( argv[2], &eptr, 0 );
    if( eptr != argv[2] ) {
      nr = l;
    }
  }

  pr( NL "I2C Recv from " );  pr_h( addr ); pr( " nr= " ); pr_d( nr ); pr( NL );

  RESET_I2C;

  uint16_t addr2 = addr<<1;
  HAL_StatusTypeDef rc = HAL_I2C_Master_Receive( i2ch_dbg, addr2, (uint8_t*)(&gbuf_a), nr, I2C_TO );
  I2C_PRINT_STATUS;
  if( rc == HAL_OK ) {
    dump8( gbuf_a, nr );
  }

  // i2c_print_status( i2c_dbg );

  return 0;
}
CmdInfo CMDINFO_I2C_RECV {
  "recv",  'R', cmd_i2c_recv,    "[addr [nr]] - recv from I2C (def addr=var[p])"
};

int cmd_i2c_recv_r1( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;
  if( argc < 2 ) {
    pr( "** ERR: reg required" NL );
    return -1;
  }
  uint8_t addr = (uint8_t)(user_vars['p'-'a']);
  uint8_t reg = (uint8_t) strtol( argv[1], 0, 0 );
  uint16_t n = 1;

  if( argc > 2 ) {
    char *eptr;
    long l = strtol( argv[2], &eptr, 0 );
    if( eptr != argv[2] ) {
      n = (uint16_t)( l );
    }
    if( n > sizeof(gbuf_a) ) {
      n = sizeof(gbuf_a);
    }
  }

  pr( NL "I2C recv r1 from  " );
  pr_h( addr ); pr( ":" ); pr_h( reg ); pr( " n= " ); pr_d( n );

  RESET_I2C;

  uint16_t addr2 = addr<<1;
  HAL_StatusTypeDef rc = HAL_I2C_Mem_Read( i2ch_dbg, addr2, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)(gbuf_a), n, I2C_TO );
  I2C_PRINT_STATUS;
  if( rc == HAL_OK ) {
    dump8( gbuf_a, n );
  }
  // i2c_print_status( i2c_dbg );

  return 0;
}
CmdInfo CMDINFO_I2C_RECV_R1 {
  "recv1",  0,  cmd_i2c_recv_r1, "reg [n] - recv from I2C(reg), reg_sz=1 (addr=var[p])"
};

// void i2c_print_status( DevI2C &i2c )
// {
//   int nt, err, ev;
//   err = i2c.getErr();    pr_sdx( err );
//   nt  = i2c.getNTrans(); pr_sdx( nt );
//   ev  = i2c.getLastEv(); pr_shx( ev );
// }

// TODO: combine with r1
int cmd_i2c_recv_r2( int argc, const char * const * argv )
{
  CHECK_I2C_DEV;
  if( argc < 2 ) {
    pr( "** ERR: reg required" NL );
    return -1;
  }
  uint8_t addr = (uint8_t)(user_vars['p'-'a']);
  uint16_t reg  = (uint16_t) strtol( argv[1], 0, 0 );
  uint16_t n = 1;

  if( argc > 2 ) {
    char *eptr;
    long l = strtol( argv[2], &eptr, 0 );
    if( eptr != argv[2] ) {
      n = (uint16_t)( l );
    }
    if( n > sizeof(gbuf_a) ) {
      n = sizeof(gbuf_a);
    }
  }

  pr( NL "I2C recv r2 from  " );
  pr_h( addr ); pr( ":" ); pr_h( reg ); pr( " n= " ); pr_d( n );

  RESET_I2C;

  uint16_t addr2 = addr<<1;
  HAL_StatusTypeDef rc = HAL_I2C_Mem_Read( i2ch_dbg, addr2, reg, I2C_MEMADD_SIZE_16BIT, (uint8_t*)(gbuf_a), n, I2C_TO );
  I2C_PRINT_STATUS;
  if( rc == HAL_OK ) {
    dump8( gbuf_a, n );
  }

  return 0;
}
CmdInfo CMDINFO_I2C_RECV_R2 {
  "recv2",  0,  cmd_i2c_recv_r2, "reg [n] - recv from I2C(reg), reg_sz=2 (addr=var[p])"
};

#undef CHECK_I2C_DEV
#undef I2C_PRINT_STATUS
#undef I2C_TO

