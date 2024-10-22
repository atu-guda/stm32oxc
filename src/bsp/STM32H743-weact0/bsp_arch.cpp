#include <oxc_base.h>

// really RAM_D2

static bool bsp_allocated_axi = false;

void* malloc_axi( size_t sz )
{
  if( bsp_allocated_axi || sz > (288*1024) ) {
    return nullptr;
  }
  bsp_allocated_axi = true;
  return (void*)(0x30000000);
}

void  free_axi( void* ptr )
{
  if( ptr != (void*)(0x30000000) ) {
    return;
  }
  bsp_allocated_axi = false;
}


