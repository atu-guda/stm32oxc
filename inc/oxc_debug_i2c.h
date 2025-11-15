#ifndef _OXC_DEBUG_I2C_H
#define _OXC_DEBUG_I2C_H

#include <oxc_i2c.h>

// extern I2C_HandleTypeDef *i2ch_dbg;
extern DevI2C *i2c_dbg;
extern I2CClient *i2c_client_def;


int cmd_i2c_scan( int argc, const char * const * argv );
int cmd_i2c_send( int argc, const char * const * argv );
int subcmd_i2c_send_rx( int argc, const char * const * argv, bool is2byte );
int cmd_i2c_send_r2( int argc, const char * const * argv );
int cmd_i2c_recv( int argc, const char * const * argv );

int subcmd_i2c_recv_rx( int argc, const char * const * argv, bool is2byte );
int cmd_i2c_recv_r1( int argc, const char * const * argv );
int cmd_i2c_recv_r2( int argc, const char * const * argv );
int cmd_i2c_setaddr( int argc, const char * const * argv );

// void i2c_print_status( I2C_HandleTypeDef *i2ch );

#endif

