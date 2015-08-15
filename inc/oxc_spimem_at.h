#ifndef _OXC_SPIMEM_AT_H
#define _OXC_SPIMEM_AT_H


#include <oxc_spi.h>

class DevSPIMem_AT {
  public:
   enum Cmd {
     WRSR = 1,    // (+1B status)
     PGM  = 2,    // (+3B addr, +up to endpage (%256) data, wait )
     READ = 3,    // (+3B addr, -> read .... )
     WRDI = 4,    // (write disable)
     RDSR = 5,    // (-> 1B status )
     WREN = 6,    // (write enable)
     RDID = 0x15, // (+2B 1F=Atmel, 60=64KB)
     ER_S = 0x52, // (+3B addr, Sector erase + wait )
     ER_C = 0x62  // (Chip erase)
   };
   enum sflags {
     BUSY      = 0x01,    //* busy = !RDY (1 while write/erase cycle
     WEN       = 0x02,    //* write enable
     BP0       = 0x04,    //* block protect 0
     BP1       = 0x08,    //* block protect 1
     WPEN      = 0x80,    //* write protect enable
   };

   DevSPIMem_AT( DevSPI &a_spi, int a_w_quant = 10, int a_w_max = 500 )
     : spi( a_spi ), w_quant( a_w_quant ), w_max( a_w_max )  {};

   uint8_t status();          // read status
   bool write_status( uint8_t sta ); // write status
   int write( const uint8_t *buf, uint32_t addr, int n ); // + wren
   int read( uint8_t *buf, uint32_t addr, int n );
   bool write_enable();
   bool write_disable();
   uint16_t read_id();
   bool erase_sector( uint32_t addr );
   bool erase_chip();
   bool wait_ready();

  protected:
   DevSPI &spi;
   int  w_quant; // wait quant in ms, def=10ms
   int  w_max;   // max time to wait in quants, def=5s
};

#endif

