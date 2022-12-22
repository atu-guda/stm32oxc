#ifndef _OXC_IO_H
#define _OXC_IO_H

#include <oxc_base.h>
#include <oxc_ev.h>

// abstract IO classes (protocols)

class DevOut {
  public:
   virtual void reset_out() = 0;
   virtual int  write( const char *s, int l ) = 0; //* return number of outputed char or -1
   virtual int  puts( const char *s ) = 0;         //* return number of outputed char or -1
   virtual int  putc( char b ) = 0;
   virtual void flush_out()  = 0;
   virtual const char* getBuf() const { return nullptr; }
};

class DevIn {
  public:
   virtual void reset_in() = 0;
   virtual Chst tryGet() = 0;
   virtual unsigned tryGetLine( char *d, unsigned max_len ) = 0 ;
   virtual Chst getc( int w_tick = 0 ) = 0;
   virtual int read( char *s, int l, int w_tick = 0 );
   virtual void unget( char c ) = 0;
   virtual const char* getInBuf() const { return nullptr; }
   virtual unsigned getInBufSize() const { return 0; }
};

// declaration of base io functions

// for devio, but may be defined by other means
Chst getChar( int fd, int w_tick );
Chst tryGet( int fd );
unsigned tryGetLine( int fd, char *d, unsigned max_len );
int unget( int fd, char c );
int ungets( int fd, const char *s );
int reset_in( int fd );
int reset_out( int fd );
int reset_io( int fd );
int write( int fd, const char *s, int l );
int pr( const char *s, int fd = 1 );
int prl( const char *s, unsigned l, int fd = 1 );
int prl1( const char *s, unsigned l ); // fd == 1 for used as flush funcs



#endif

