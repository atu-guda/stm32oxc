#ifndef _OXC_BARTEXT_H
#define _OXC_BARTEXT_H

// ------------------------ BarHText ----------------------------------

class BarHText {
  public:
   BarHText( int x, int y, int w, int v_min, int v_max, int a_fd = 1 );
   void draw( int v );
   void setUseCodes( bool uc ) { useCodes = uc; };
   void setDrawZero( bool dz ) { drawZero = dz; };
   void setCharSpace( char cs ) { c_space = cs; };
   void setCharBar( char cb ) { c_bar = cb; };
   void setCharZero( char cz ) { c_zero = cz; };
  protected:
   int x0, y0, w0, vmin, vmax, fd, i0;
   bool useCodes = true;
   bool drawZero = true;
   char c_space = '.', c_bar = 'W', c_zero = '*', c_zero_e = '?';
};

#endif

