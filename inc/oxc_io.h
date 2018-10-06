#ifndef _OXC_IO_H
#define _OXC_IO_H

// abstract IO classes (protocols)

class DevOut {
  public:
   virtual void reset_out() = 0;
   virtual int  write( const char *s, int l ) = 0;
   virtual int  puts( const char *s ) = 0;
   virtual int  putc( char b ) = 0;
   virtual void flush_out()  = 0;
};

class DevIn {
  public:
   virtual void reset_in() = 0;
   virtual Chst tryGet() = 0;
   virtual unsigned tryGetLine( char *d, unsigned max_len ) = 0 ;
   virtual Chst getc( int w_tick = 0 ) = 0;
   virtual int read( char *s, int l, int w_tick = 0 );
   virtual void unget( char c ) = 0;
};

// declaration of base io functions

// for devio, but may be defined by other means
Chst getChar( int fd, int w_tick );
Chst tryGet( int fd );
unsigned tryGetLine( int fd, char *d, unsigned max_len );
int write( int fd, const char *s, int l );
int pr( const char *s, int fd = 1 );
int prl( const char *s, unsigned l, int fd = 1 );
int prl1( const char *s, unsigned l ); // fd == 1 for used as flush funcs


extern "C" {
// defines from newlib: hidden by some macros
int  asprintf( char **__restrict, const char *__restrict, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
int  vasprintf( char **, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
int  asiprintf( char **, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
char *  asniprintf( char *, size_t *, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 3, 4)));
char *  asnprintf( char *__restrict, size_t *__restrict, const char *__restrict, ... )
               _ATTRIBUTE( (__format__( __printf__, 3, 4)));
int  diprintf( int, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
int  fiprintf( FILE *, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
int  fiscanf( FILE *, const char *, ... )
               _ATTRIBUTE( (__format__( __scanf__, 2, 3)));
int  iprintf( const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 1, 2)));
int  iscanf( const char *, ... )
               _ATTRIBUTE( (__format__( __scanf__, 1, 2)));
int  siprintf( char *, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
int  siscanf( const char *, const char *, ... )
               _ATTRIBUTE( (__format__( __scanf__, 2, 3)));
int  sniprintf( char *, size_t, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 3, 4)));
int  vasiprintf( char **, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
char *  vasniprintf( char *, size_t *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 3, 0)));
char *  vasnprintf( char *, size_t *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 3, 0)));
int  vdiprintf( int, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
int  vfiprintf( FILE *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
int  vfiscanf( FILE *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __scanf__, 2, 0)));
int  viprintf( const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 1, 0)));
int  viscanf( const char *, __VALIST )
               _ATTRIBUTE( (__format__( __scanf__, 1, 0)));
int  vsiprintf( char *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
int  vsiscanf( const char *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __scanf__, 2, 0)));
int  vsniprintf( char *, size_t, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 3, 0)));
};


#endif

