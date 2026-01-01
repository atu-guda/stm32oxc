#ifndef _OXC_HX711_H
#define _OXC_HX711_H

// 24-bit ADC for scales: HX711_ADC_for_weight.pdf

#include <oxc_gpio.h>


class HX711 {
  public:
   enum HX711_mode { mode_A_x128 = 0, mode_B_x32 = 1, mode_A_x64 = 2 }; // 25, 26, 27 sck pulses
   enum { rc_Ok = 0, rc_notReady = 1 };
   enum { max_wait_rdy_mcs = 100000 };
   enum { ADC_BITS = 24 };
   struct Ret {
     uint32_t rc;
     int32_t  v;
     bool isOk() const { return rc == rc_Ok; }
     int32_t value_or( int32_t def ) const { return ( rc == rc_Ok ) ? v : def; }
     int32_t value_or_0() const { return ( rc == rc_Ok ) ? v : 0; }
   };
   HX711( PortPin pn_sck, PortPin pn_dat )
     : sck( pn_sck ), dat( pn_dat, 1 ) {};
   void initHW() { sck.initHW(); dat.initHW(); }; // ??? more: state:
   Ret read( HX711_mode mode = mode_A_x128 );
   void delay_hx() const { delay_bad_mcs( 1 ); };
   void tick_sck() { sck.set(); delay_hx(); sck.reset(); delay_hx(); };
  protected:
   PinOut sck;
   PinsIn dat;
};


#endif

