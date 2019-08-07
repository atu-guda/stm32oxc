#ifndef _OXC_ONEWIRE_H
#define _OXC_ONEWIRE_H

#include <oxc_gpio.h>

#define d_mcs delay_bad_mcs


class OneWire {
  public:
   enum {
     T_W1_L =  6, T_W1_H = 64,
     T_W0_L = 60, T_W0_H = 10,
     T_R_L = 6, T_R_H1 = 9, T_R_H2 = 55,
     T_RST_L = 480, T_RST_H1 = 70, T_RST_H2 = 410,
     CMD_SEARCH_ROM = 0xF0,
     CMD_READ_ROM = 0x33,
     CMD_MATCH_ROM = 0x55,
     CMD_SKIP_ROM = 0xCC,
     CMD_WRITE_SPAD = 0x4E,
     CMD_READ_SPAD = 0xBE
   };
   OneWire( IoPin  &a_p )
     : p( a_p ) {};
   void initHW() { p.sw1(); err = 0; };
   void eot()    { p.sw1(); };

   bool reset() {
     p.sw0(); d_mcs( T_RST_L ); p.sw1(); d_mcs( T_RST_H1 );
     bool r = ! p.rw_raw(); d_mcs( T_RST_H2 );
     if( !r ) { err  = 1; }
     return r;
   };
   void w0() { p.sw0(); d_mcs( T_W0_L ); p.sw1(); d_mcs( T_W0_H ); };
   void w1() { p.sw0(); d_mcs( T_W1_L ); p.sw1(); d_mcs( T_W1_H ); };
   void write1bit( bool b ) { if( b ) w1(); else w0(); };
   uint8_t read1bit() {
     p.sw0(); d_mcs( T_R_L ); p.sw1(); d_mcs( T_R_H1 );
     uint8_t r = p.rw_raw(); d_mcs( T_R_H2 );
     return r;
   };
   void write1byte( uint8_t w ) { write_buf( &w, 1 ); };
   uint8_t read1byte(){ uint8_t r; read_buf( &r, 1 ); return r; };
   void write_buf( const uint8_t *b, uint16_t l );
   void read_buf( uint8_t *b, uint16_t l );

   bool gcmd( const uint8_t *addr, uint8_t cmd, // generic command
       const uint8_t *snd, uint8_t s_sz, uint8_t *rcv, uint16_t r_sz  );

   bool searchRom( const uint8_t *snd, uint8_t *rcv, uint16_t r_sz  );
   bool readRom( uint8_t *rcv, uint16_t r_sz = 8  );
   bool matchRom( const uint8_t *addr, uint8_t cmd, uint8_t *rcv, uint16_t r_sz );
   bool skipRom( uint8_t cmd, uint8_t *rcv, uint16_t r_sz );
   static uint8_t calcCrc( const uint8_t *b, uint16_t l );
   void set_check_crc( bool a_set ) { check_crc = a_set; }
   int getErr() const { return err; }
  protected:
   IoPin &p;
   int err = 0;
   bool check_crc = true;
};


#endif

