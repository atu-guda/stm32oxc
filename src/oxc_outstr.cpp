/*
 * =====================================================================================
 *       Filename:  oxc_outstr.cpp
 *    Description:  Implementation of the OutStr class
 *
 *        Created:  2019-05-22 21:28:11
 *
 *         Author:  Anton Guda (atu), atu@nmetau.edu.ua
 *   Organization:  NMetAU.ITS
 * =====================================================================================
 */

#include <cstring>

#include <oxc_outstr.h>

using namespace std;

void OutStr::reset_out()
{
  sz = 0; buf[0] = '\0';
}

int  OutStr::write( const char *s, int l )
{
  if( !s ) {
    return -1;
  }
  if( l < 1 ) {
    return 0;
  }

  int nw = 0;
  char *t = buf + sz;
  for( int i=0; i<l && sz < bsz-1; ++i ) {
    *t++ = *s++;
    ++sz;
  }
  *t = '\0';

  return nw;
}

int  OutStr::puts( const char *s )
{
  if( !s ) { return 0; }
  return write( s, strlen(s) );
}

int  OutStr::putc( char b )
{
  return write( &b, 1 );
}

void OutStr::flush_out()
{
  // nothing here
}


