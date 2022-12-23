#ifndef _OXC_OSFUN_H
#define _OXC_OSFUN_H

#ifdef __cplusplus
 extern "C" {
#endif

void _exit( int rc );

// osfuncs, real or not
struct stat;
struct tms;
char* _sbrk ( int incr );
int _getpid(void);
int _kill( int pid, int sig );
void _exit( int status );
int _read( int fd, char *buf, int len );
int _write( int fd, const char *buf, int len );
int _close( int fd );
int _fstat( int fd, struct stat *st );
int _isatty( int fd );
int _lseek( int fd, int ptr, int whence );
int _open( const char *path, int flags, ... );
int _wait( int *status );
int _unlink( char *name );
int _times( struct tms *buf );
int _stat( const char *file, struct stat *st );
int _link( const char *oldname, const char *newname );
int _fork(void);
int _execve( char *name, char **argv, char **env );

#ifdef __cplusplus
}
#endif


#endif

