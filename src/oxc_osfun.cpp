#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <oxc_base.h>
#include <oxc_osfun.h>

#ifndef OXC_FAKE_IO
#include <oxc_devio.h>
#endif



extern "C" {

char* _sbrk ( int incr )
{
  char *prev_heap;

  prev_heap = __heap_top;

  // even decrease - newlib malloc/free use it!
  incr += 3;
  incr &= ~3; // align to 4 bytes

  if( (unsigned)(__heap_top) + incr + 128 >= (unsigned)(__get_MSP()) ) {
    errno = ENOMEM;
    // dbg_val0 = (unsigned)(__heap_top) + incr + 128;
    // dbg_val1 = incr;
    // dbg_val3 = __get_MSP();
    return (char*)(-1);
  }

  __heap_top += incr;

  return prev_heap;
}


#ifdef OXC_FAKE_IO

int _read( int fd, char *buf, int len )
{
  errno = EBADF;
  return -1;
}

int _write( int fd, const char *buf, int len )
{
  errno = EBADF;
  return -1;
}


int _close( int fd )
{
  return -1;
}


int _fstat( int fd, struct stat *st )
{
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty( int fd )
{
  return 0;
}

int _lseek( int fd, int ptr, int whence )
{
  return -1;
}

int _open( const char *path, int flags, ... )
{
  errno = ENFILE;
  return -1;
}

#else
// real implementation, not all for now

#define COMMON_FD_TEST(fd) \
  if( fd < 0 || fd > DEVIO_MAX || !devio_fds[fd] ) {\
    errno = ENFILE; \
    return -1; \
  }

int _read( int fd, char *buf, int len )
{
  COMMON_FD_TEST( fd );
  return devio_fds[fd]->read( buf, len );
}

int _write( int fd, const char *buf, int len )
{
  COMMON_FD_TEST( fd );
  return devio_fds[fd]->write( buf, len );
}


int _close( int fd )
{
  COMMON_FD_TEST( fd );
  devio_fds[fd]->reset();
  return 0;
}


int _fstat( int fd, struct stat *st )
{
  COMMON_FD_TEST( fd );
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty( int fd )
{
  COMMON_FD_TEST( fd );
  return ( fd < 3 );
}

int _lseek( int fd, int ptr, int whence )
{
  COMMON_FD_TEST( fd );
  return -1;
}

int _open( const char *path, int flags, ... )
{
  errno = ENFILE;
  return -1;
}

#endif

int _wait( int *status )
{
  errno = ECHILD;
  return -1;
}

int _unlink( char *name )
{
  errno = ENOENT;
  return -1;
}

int _times( struct tms *buf )
{
  return -1;
}

int _stat( const char *file, struct stat *st )
{
  st->st_mode = S_IFCHR;
  return 0;
}

int _link( const char *oldname, const char *newname )
{
  errno = EMLINK;
  return -1;
}

void abort(void)
{
  for(;;);
}


int _getpid(void)
{
  return 1;
}

int _kill( int pid, int sig )
{
  errno = EINVAL;
  return -1;
}

int _fork(void)
{
  errno = EAGAIN;
  return -1;
}

int _execve( char *name, char **argv, char **env )
{
  errno = ENOMEM;
  return -1;
}


} // extern "C"

namespace __gnu_cxx {
  void __verbose_terminate_handler() {
    for(;;);
  }
}

extern "C" void __cxa_pure_virtual() {
  for(;;);
}


void* operator new( size_t size ) {
  return malloc( size );
}


void* operator new[]( size_t size ) {
  return malloc( size );
}


void operator delete( void *p ) {
  free( p );
}

void operator delete[]( void *p ) {
  free( p );
}


void operator delete( void *p, unsigned /*sz*/ ) {
  free( p );
}

void operator delete[]( void *p, unsigned /*sz*/ ) {
  free( p );
}

