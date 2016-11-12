#include <stdlib.h>
#include <errno.h>
#include <oxc_base.h>


extern  const int _sdata, _edata, _sbss, _ebss, _end, _estack;
char* __heap_top = (char*)(&_end);

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

void abort(void)
{
  for(;;);
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

