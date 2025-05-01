#ifndef _OXC_DEBUG_I2C_H
#define _OXC_DEBUG_I2C_H

#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_i2c.h>

// extern I2C_HandleTypeDef *i2ch_dbg;
extern DevI2C *i2c_dbg;
extern I2CClient *i2c_client_def;


int cmd_i2c_scan( int argc, const char * const * argv );
extern const CmdInfo CMDINFO_I2C_SCAN;
int cmd_i2c_send( int argc, const char * const * argv );
extern const CmdInfo CMDINFO_I2C_SEND;
int subcmd_i2c_send_rx( int argc, const char * const * argv, bool is2byte );
int cmd_i2c_send_r1( int argc, const char * const * argv );
extern const CmdInfo CMDINFO_I2C_SEND_R1;
int cmd_i2c_send_r2( int argc, const char * const * argv );
extern const CmdInfo CMDINFO_I2C_SEND_R2;
int cmd_i2c_recv( int argc, const char * const * argv );
extern const CmdInfo CMDINFO_I2C_RECV;

int subcmd_i2c_recv_rx( int argc, const char * const * argv, bool is2byte );
int cmd_i2c_recv_r1( int argc, const char * const * argv );
extern const CmdInfo CMDINFO_I2C_RECV_R1;
int cmd_i2c_recv_r2( int argc, const char * const * argv );
extern const CmdInfo CMDINFO_I2C_RECV_R2;
int cmd_i2c_setaddr( int argc, const char * const * argv );
extern const CmdInfo CMDINFO_I2C_SETADDR;

#define DEBUG_I2C_CMDS \
  &CMDINFO_I2C_SCAN, \
  &CMDINFO_I2C_SEND, \
  &CMDINFO_I2C_SEND_R1, \
  &CMDINFO_I2C_SEND_R2, \
  &CMDINFO_I2C_RECV, \
  &CMDINFO_I2C_RECV_R1, \
  &CMDINFO_I2C_RECV_R2, \
  &CMDINFO_I2C_SETADDR

// void i2c_print_status( I2C_HandleTypeDef *i2ch );

#endif

