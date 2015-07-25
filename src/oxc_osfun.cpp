#include <stdlib.h>
#include <oxc_base.h>


extern int  _end;
extern "C" {
  char* _sbrk ( int incr ) {

    static unsigned char *heap = NULL;
    unsigned char *prev_heap;

    if (heap == NULL) {
      heap = (unsigned char *)&_end;
    }
    prev_heap = heap;
    /* check removed to show basic approach */

    heap += incr;

    return (char*) prev_heap;
  }
}

extern "C" {
void abort(void)
{
  for(;;) ;
}
}

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

void* operator new( size_t, void *ptr ) {
  return ptr;
}

void* operator new[]( size_t size ) {
  return malloc( size );
}

void* operator new[]( size_t, void *ptr ) {
  return ptr;
}

void operator delete( void *p ) {
  free( p );
}

void operator delete[]( void *p ) {
  free( p );
}

