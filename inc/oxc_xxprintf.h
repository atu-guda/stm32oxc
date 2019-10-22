#ifndef _OXC_XXPRINTF_H
#define _OXC_XXPRINTF_H


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

