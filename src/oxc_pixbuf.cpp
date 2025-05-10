#include <cstring>
#include <algorithm>

using namespace std;

#include <oxc_miscfun.h>
#include <oxc_pixbuf.h>

PixBuf::PixBuf( uint16_t a_width, uint16_t a_height, uint16_t a_bpp )
  : width( a_width ), height( a_height ), bpp( a_bpp ),
    sz( uint32_t(width) * height * bpp / 8 ),
    xscr( new uint8_t[sz+PRE_BUF] ),
    scr( xscr+PRE_BUF )
{
}

PixBuf::PixBuf( const PixBuf &r )
  : width(r.width), height(r.height), bpp(r.bpp), sz(r.sz),
    xscr( new uint8_t[sz+PRE_BUF] ),
    scr( xscr+PRE_BUF )
{
  memcpy( xscr, r.xscr, sz+PRE_BUF );
}

PixBuf::~PixBuf()
{
  delete[] xscr; xscr = nullptr; scr = nullptr;
}

void PixBuf::pix(  uint16_t x, uint16_t y, uint32_t col )
{
  if( x >= width || y >= height ) {
    return;
  }
  pixx( x, y, col );
}

void PixBuf::rect( uint16_t x1,  uint16_t y1, uint16_t x2, uint16_t y2, uint32_t col )
{
  hline( x1, y1, x2, col );
  hline( x1, y2, x2, col );
  vline( x1, y1, y2, col );
  vline( x2, y1, y2, col );
}

void PixBuf::box(  uint16_t x1,  uint16_t y1, uint16_t x2, uint16_t y2, uint32_t col )
{
  // fallback realization
  for( uint16_t y=y1; y<=y2; ++y ) {
    hline( x1, y, x2, col );
  }
}

void PixBuf::line(  uint16_t x1,  uint16_t y1, uint16_t x2, uint16_t y2, uint32_t col )
{
  uint16_t dx = abs( x2 - x1 );
  uint16_t dy = abs( y2 - y1 );
  if( dx == 0 ) {
    return vline( x1, y1, y2, col );
  }
  if( dy == 0 ) {
    return hline( x1, y1, x2, col );
  }

  int d = 0;
  if( dx > dy ) { // -------------- step by x

    if( x2 < x1 ) {
      swap( x1, x2 ); swap( y1, y2 );
    };
    int dlt = sign( (int)(y2) - (int)(y1) );
    int dlterr = 2 * dy;
    int cy = y1;
    for( uint16_t cx=x1; cx<=x2; ++cx ) {
      pix( cx, cy, col );
      d += dlterr;
      if( d >= dx ) {
        cy += dlt;
        d -= 2 * dx;
      }
    }

  } else {        // -------------- step by x

    if( y2 < y1 ) {
      swap( x1, x2 ); swap( y1, y2 );
    };
    int dlt = sign( (int)(x2) - (int)(x1) );
    int dlterr = 2 * dx;
    int cx = x1;
    for( uint16_t cy=y1; cy<=y2; ++cy ) {
      pix( cx, cy, col );
      d += dlterr;
      if( d >= dy ) {
        cx += dlt;
        d -= 2 * dy;
      }
    }
  }

}

void PixBuf::circle( uint16_t x0,  uint16_t y0, uint16_t r, uint32_t col )
{
  uint16_t xc = 0;
  int dlt = 1 - 2 * r;
  int d = 0, yc = r;
  while( yc >=0 ) {
    pix( x0+xc, y0+yc, col );
    pix( x0+xc, y0-yc, col );
    pix( x0-xc, y0+yc, col );
    pix( x0-xc, y0-yc, col );
    d = 2 * ( dlt + yc ) - 1;
    if( ( dlt < 0) && ( d <= 0) ) {
      dlt += 2 * ++xc + 1;
      continue;
    }
    d = 2 * ( dlt - xc ) - 1;
    if( ( dlt > 0) && (d > 0) ) {
      dlt += 1 - 2 * --yc;
      continue;
    }
    ++xc;
    dlt += 2 * (xc - yc);
    --yc; // w/o = box with rounded corners
  }

}

void PixBuf::fillCircle( uint16_t x0,  uint16_t y0, uint16_t r, uint32_t col )
{
  uint16_t xc = 0;
  int dlt = 1 - 2 * r;
  int d = 0, yc = r;
  while( yc >=0 ) {
    hline( x0-xc, y0-yc, x0+xc, col );
    hline( x0-xc, y0+yc, x0+xc, col );
    d = 2 * ( dlt + yc ) - 1;
    if( ( dlt < 0) && ( d <= 0) ) {
      dlt += 2 * ++xc + 1;
      continue;
    }
    d = 2 * ( dlt - xc ) - 1;
    if( ( dlt > 0) && (d > 0) ) {
      dlt += 1 - 2 * --yc;
      continue;
    }
    ++xc;
    dlt += 2 * (xc - yc);
    --yc; // w/o = box with rounded corners
  }

}

void PixBuf::outChar( uint16_t x0,  uint16_t y0, char c, const sFONT *fnt, uint32_t col )
{
  if( !fnt ) { return; }
  uint16_t height = fnt->Height, width = fnt->Width, w8 = ( width + 7 ) / 8;
  uint16_t ofs = 8 * w8 - width;
  const uint8_t *cdata = & ( fnt->table[ (c-' ') * height * w8 ] );
  uint32_t line=0;

  for( int i=0; i < height; ++i ) {
    const uint8_t *pcd = ( (const uint8_t *)(cdata) + w8 * i );

    switch( w8 ) {
      case 1:
        line =  pcd[0];  break;

      case 2:
        line =  (pcd[0]<< 8) | pcd[1];  break;

      case 3:
      default:
        line = ( pcd[0]<< 16 ) | ( pcd[1]<< 8 ) | pcd[2];  break;
    }

    for( int j = 0; j < width; ++j ) {
      if( line & (1 << ( width- j + ofs - 1)) ) {
        pix( x0 + j, y0, col );
      }
    }
    ++y0;
  }
}

void PixBuf::outStr( uint16_t x0, uint16_t y0, const char *s, const sFONT *fnt, uint32_t col )
{
  if( !s || !fnt ) { return; }
  uint16_t xc = x0, w = fnt->Width;
  while( *s ) {
    outChar( xc, y0, *s, fnt, col );
    ++s; xc += w;
  }

}

void PixBuf::outStrBox( uint16_t x0,  uint16_t y0, const char *s, const sFONT *fnt,
                        uint32_t col, uint32_t bg_col, uint32_t brd_col, uint16_t flg )
{
  if( !fnt || !s ) { return; }
  int l = strlen( s );
  uint16_t w = fnt->Width, h = fnt->Height;
  uint16_t r = x0 + w * l + 1, b = y0 + h;
  if( flg | STRBOX_BG ) {
    box( x0-1, y0-1, r, b, bg_col );
  }
  if( flg | STRBOX_BORDER ) {
    rect( x0-2, y0-2, r+1, b+1, brd_col );
  }
  outStr( x0, y0, s, fnt, col );
}


