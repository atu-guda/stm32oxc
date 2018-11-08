#include <string.h>
#include <oxc_console.h>
#include <oxc_bartext.h>


BarHText::BarHText( int x, int y, int w, int v_min, int v_max, int a_fd  )
  : x0( x ), y0( y ), w0( w-2 ), vmin( v_min ), vmax( v_max ), fd( a_fd ),
    i0( ( 0 - vmin ) * ( w0-1 ) / ( vmax - vmin ) )
{
  c_zero_e = c_zero;
  if( i0 < 0 )   { i0 = 0;    c_zero_e = '<'; };
  if( i0 >= w0 ) { i0 = w0-1; c_zero_e = '>'; };
}

void BarHText::draw( int v )
{
  if( w0 < 3 ) { return; };
  char lbra = '[', rbra = ']';
  if( v < vmin ) { v = vmin; lbra = '$'; };
  if( v > vmax ) { v = vmax; rbra = '$'; };

  int iv = ( v - vmin ) * ( w0 - 1 ) / ( vmax - vmin );
  char zero_char = c_zero_e;

  int lb = i0, rb = iv;
  bool left_zero = true;
  if( lb > rb ) { int t = lb; lb = rb; rb = t; left_zero = false; }

  if( useCodes ) {
    term_save_cpos( fd );
    term_set_xy( x0, y0, fd );
  }


  pr_c( lbra, fd );
  int i; // not in loop
  for( i=0; i<lb; ++i ) {
    pr_c( c_space, fd );
  }
  pr_c( ( left_zero && drawZero ) ? zero_char : c_space ); ++i;
  for( /* */; i<rb; ++i ) {
    pr_c( c_bar, fd );
  }
  pr_c( ( !left_zero && drawZero ) ? zero_char : c_bar ); ++i;
  for( /* */; i<w0; ++i ) {
    pr_c( c_space, fd );
  }
  pr_c( rbra, fd );

  if( useCodes ) {
    term_rest_cpos( fd );
  }
}

