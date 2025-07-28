#include <oxc_spi.h>


// --- local commands;
int cmd_send1recvN_spi( int argc, const char * const * argv );
extern CmdInfo CMDINFO_S1RN;
int cmd_sendr_spi( int argc, const char * const * argv );
extern CmdInfo CMDINFO_SENDR;
int cmd_duplex_spi( int argc, const char * const * argv );
extern CmdInfo CMDINFO_DUPLEX;
int cmd_recv_spi( int argc, const char * const * argv );
extern CmdInfo CMDINFO_RECV;
int cmd_reset_spi( int argc, const char * const * argv );
extern CmdInfo CMDINFO_RESETSPI;
int cmd_sendloop_spi( int argc, const char * const * argv );
extern CmdInfo CMDINFO_SENDLOOPSPI;


#define DEBUG_SPI_CMDS  \
  &CMDINFO_S1RN,        \
  &CMDINFO_SENDR,       \
  &CMDINFO_RECV,        \
  &CMDINFO_DUPLEX,      \
  &CMDINFO_SENDLOOPSPI, \
  &CMDINFO_RESETSPI

extern PinOut dbg_pin;
extern PinOut nss_pin;
extern SPI_HandleTypeDef spi_h;
extern DevSPI spi_d;


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

