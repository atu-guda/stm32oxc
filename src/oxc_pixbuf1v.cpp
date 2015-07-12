#include <cstring>
#include <algorithm>

#include <oxc_pixbuf1v.h>

using namespace std;


void PixBuf1V::fillAll( uint32_t col )
{
  uint8_t v = col ? 0xFF : 0;
  memset( scr, v, sz );
}

void PixBuf1V::pixx( uint16_t x, uint16_t y, uint32_t col )
{
  uint32_t ofs = xy2ofs( x, y);
  uint8_t midx = xy2midx( x, y );
  if( col ) {
    scr[ofs] |= msk_set[midx];
  } else {
    scr[ofs] &= msk_uns[midx];
  }
}


void PixBuf1V::hline( uint16_t x1, uint16_t y,  uint16_t x2, uint32_t col )
{
  if( x2 < x1 ) {
    swap( x1, x2 );
  };
  if( x1 >= width || y >= height ) {
    return;
  }
  if( x2 >= width ) {
    x2 = width-1;
  }
  uint32_t ofs = xy2ofs(  x1, y );
  uint8_t midx = xy2midx( x1, y );
  uint16_t n = x2 - x1;

  if( col ) {
    uint8_t m = msk_set[midx];
    for( uint16_t i=0; i<n; ++i, ++ofs ) {
      scr[ofs] |= m;
    }
  } else {
    uint8_t m = msk_uns[midx];
    for( uint16_t i=0; i<n; ++i, ++ofs ) {
      scr[ofs] &= m;
    }
  }
}

void PixBuf1V::vline( uint16_t x,  uint16_t y1, uint16_t y2, uint32_t col )
{
  if( y2 < y1 ) {
    swap( y1, y2 );
  };
  if( x >= width || y1 >= height ) {
    return;
  }
  if( y2 >= height ) {
    y2 = height - 1;
  }
  OfsData od;
  od.ofs1  = xy2ofs( x, y1 );
  uint8_t midx1 = xy2midx( x, y1 );
  od.m1    = msk_l1[midx1];
  od.ofs2  = xy2ofs( x, y2 );
  uint8_t midx2 = xy2midx( x, y2 );
  od.m2    = msk_l2[midx2];

  if( od.ofs1 == od.ofs2 ) { // single segment
    od.m1 &= od.m2;
  }
  if( col ) {
    vline1( od );
  } else {
    vline0( od );
  }
}

void PixBuf1V::vline0( const OfsData &od )
{
  scr[od.ofs1] &= ~od.m1;
  if( od.ofs1 == od.ofs2 ) { // single segment
    return;
  }

  scr[od.ofs2] &= ~od.m2;
  for( uint32_t o = od.ofs1+width; o < od.ofs2; o+=width ) {
    scr[o] = 0;
  }

}

void PixBuf1V::vline1( const OfsData &od )
{
  scr[od.ofs1] |= od.m1;
  if( od.ofs1 == od.ofs2 ) { // single segment
    return;
  }

  scr[od.ofs2] |= od.m2;
  for( uint32_t o = od.ofs1+width; o < od.ofs2; o+=width ) {
    scr[o] = 0xFF;
  }
}


void PixBuf1V::box(  uint16_t x1,  uint16_t y1, uint16_t x2, uint16_t y2, uint32_t col )
{
  if( x2 < x1 ) {
    swap( x1, x2 );
  };
  if( y2 < y1 ) {
    swap( y1, y2 );
  };
  if( x1 >= width || y1 >= height ) {
    return;
  }
  if( x2 >= width ) {
    x2 = width-1;
  }
  if( y2 >= height ) {
    y2 = height-1;
  }
  OfsData od;
  od.ofs1  = xy2ofs( x1, y1 );
  uint8_t midx1 = xy2midx( x1, y1 );
  od.m1    = msk_l1[midx1];
  od.ofs2  = xy2ofs( x1, y2 );
  uint8_t midx2 = xy2midx( x1, y2 );
  od.m2    = msk_l2[midx2];
  uint16_t n = x2 - x1;

  if( od.ofs1 == od.ofs2 ) { // single segment
    od.m1 &= od.m2;
  }
  if( col ) {
    box1( od, n );
  } else {
    box0( od, n );
  }
}

void PixBuf1V::box0( const OfsData &od, uint16_t n )
{
  for( uint16_t i=0; i<n; ++i ) {
    scr[od.ofs1+i] &= ~od.m1;
  }
  if( od.ofs1 == od.ofs2 ) { // single segment
    return;
  }

  for( uint16_t i=0; i<n; ++i ) {
    scr[od.ofs2+i] &= ~od.m2;
    for( uint32_t o = od.ofs1+width; o < od.ofs2; o+=width ) {
      scr[o+i] = 0;
    }
  }
}

void PixBuf1V::box1( const OfsData &od, uint16_t n )
{
  for( uint16_t i=0; i<n; ++i ) {
    scr[od.ofs1+i] |= od.m1;
  }
  if( od.ofs1 == od.ofs2 ) { // single segment
    return;
  }

  for( uint16_t i=0; i<n; ++i ) {
    scr[od.ofs2+i] |= od.m2;
    for( uint32_t o = od.ofs1+width; o < od.ofs2; o+=width ) {
      scr[o+i] = 0xFF;
    }
  }
}

