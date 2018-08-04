#ifndef _OXC_IO_H
#define _OXC_IO_H

// declaration of base io functions

// for devio, but may be defined by other means
int recvByte( int fd, char *s, int w_tick = 0 );
int sendBlock( int fd, const char *s, int l );
int pr( const char *s, int fd = 1 );
int prl( const char *s, unsigned l, int fd = 1 );
int prl1( const char *s, unsigned l ); // fd == 1 for used as flush funcs


#endif

