#include <oxc_spi.h>


// --- local commands;
int cmd_spi_s1rn( int argc, const char * const * argv );
int cmd_spi_sendr( int argc, const char * const * argv );
int cmd_spi_duplex( int argc, const char * const * argv );
int cmd_spi_recv( int argc, const char * const * argv );
int cmd_spi_reset( int argc, const char * const * argv );
int cmd_spi_sendloop( int argc, const char * const * argv );

extern PinOut dbg_pin;
extern PinOut nss_pin;
extern SPI_HandleTypeDef spi_h;
extern DevSPI spi_d;


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

