#ifndef _OXC_PIXBUF1V_H
#define _OXC_PIXBUF1V_H

#include <oxc_pixbuf.h>

class PixBuf1V : public PixBuf {
  public:
   PixBuf1V( uint16_t a_w, uint16_t a_h )
     : PixBuf( a_w, a_h, 1 ) {};
   virtual uint32_t xy2ofs( uint16_t x, uint16_t y ) override {
     return x + (uint32_t)( y >> 3 ) * width;
   }
   virtual uint8_t xy2midx( uint16_t x UNUSED_ARG, uint16_t y ) override {
     return (uint8_t)( y & 0x07 );
   }
   virtual void fillAll( uint32_t col ) override;
   virtual void pixx( uint16_t x, uint16_t y, uint32_t col ) override; // w/o control: private?
   // virtual void pix(  uint16_t x, uint16_t y, uint32_t col ) override;  // with control
   virtual void hline( uint16_t x1, uint16_t y,  uint16_t x2, uint32_t col ) override;
   virtual void vline( uint16_t x,  uint16_t y1, uint16_t y2, uint32_t col ) override;
   // virtual void rect( uint16_t x1,  uint16_t y1, uint16_t x2, uint16_t y2, uint32_t col ) override;
   virtual void box(  uint16_t x1,  uint16_t y1, uint16_t x2, uint16_t y2, uint32_t col ) override;

   struct OfsData {
     uint32_t ofs1;
     uint8_t m1;
     uint32_t ofs2;
     uint8_t m2;
   };
  protected:
   void vline0( const OfsData &od );
   void vline1( const OfsData &od );
   void box0( const OfsData &od, uint16_t n );
   void box1( const OfsData &od, uint16_t n );
  protected:
   const uint8_t msk_set[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
   const uint8_t msk_uns[8] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };
   const uint8_t msk_l1[8]  = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 };
   const uint8_t msk_l2[8]  = { 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };
};


#endif

// vim: path=.,/usr/share/stm32cube/inc
