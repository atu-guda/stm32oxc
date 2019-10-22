#ifndef _OXC_PIXBUF_H
#define _OXC_PIXBUF_H

#include <fonts.h>

#include <oxc_base.h>

//* abstract pixel buffer: unknown bit depth and videoram structure
class PixBuf {
  public:
   enum {
     PRE_BUF = 4, // bytes before real buffer: for transfer cmd
     STRBOX_BG = 1,
     STRBOX_BORDER = 2,
     STRBOX_ALL = STRBOX_BG | STRBOX_BORDER
   };
   PixBuf( uint16_t a_width, uint16_t a_height, uint16_t a_bpp );
   PixBuf( const PixBuf &r );
   ~PixBuf();
   uint8_t* fb() { return scr; }
   uint8_t* xfb() { return xscr; }

   virtual uint32_t xy2ofs( uint16_t x, uint16_t y ) = 0;
   virtual uint8_t xy2midx( uint16_t x UNUSED_ARG, uint16_t y ) = 0;
   virtual void fillAll( uint32_t col ) = 0;
   virtual void pixx( uint16_t x, uint16_t y, uint32_t col ) = 0; // w/o control: private?
   virtual void pix(  uint16_t x, uint16_t y, uint32_t col );  // with control
   virtual void hline( uint16_t x1, uint16_t y,  uint16_t x2, uint32_t col ) = 0;
   virtual void vline( uint16_t x,  uint16_t y1, uint16_t y2, uint32_t col ) = 0;
   virtual void rect( uint16_t x1,  uint16_t y1, uint16_t x2, uint16_t y2, uint32_t col );
   virtual void box(  uint16_t x1,  uint16_t y1, uint16_t x2, uint16_t y2, uint32_t col ) = 0;
   virtual void line(  uint16_t x1,  uint16_t y1, uint16_t x2, uint16_t y2, uint32_t col );
   virtual void circle( uint16_t x0,  uint16_t y0, uint16_t r, uint32_t col );
   virtual void fillCircle( uint16_t x0,  uint16_t y0, uint16_t r, uint32_t col );
   virtual void outChar( uint16_t x0,  uint16_t y0, char c, const sFONT *fnt, uint32_t col );
   virtual void outStr( uint16_t x0,  uint16_t y0, const char *s, const sFONT *fnt, uint32_t col );
   virtual void outStrBox( uint16_t x0,  uint16_t y0, const char *s, const sFONT *fnt,
                           uint32_t col, uint32_t bg_col, uint32_t brd_col, uint16_t flg = STRBOX_BG );
  protected:
   uint16_t width, height, bpp;
   uint32_t sz; // in bytes;
   uint8_t *xscr;
   uint8_t *scr;
};

#endif

// vim: path=.,/usr/share/stm32cube/inc
