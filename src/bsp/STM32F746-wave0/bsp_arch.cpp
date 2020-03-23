#include <oxc_base.h>

#ifdef USE_OXC_SDRAM
#include <board_sdram.h>
#endif

//---------------------------------------------------

#ifdef USE_OXC_SDRAM
static bool bsp_allocated_fmc = false;

void* malloc_fmc( size_t sz )
{
  if( bsp_allocated_fmc || sz > SDRAM_DEVICE_SIZE ) {
    return nullptr;
  }
  bsp_allocated_fmc = true;
  return (void*)(SDRAM_BANK_ADDR);
}

void  free_fmc( void* ptr )
{
  if( ptr != (void*)(SDRAM_BANK_ADDR) ) {
    return;
  }
  bsp_allocated_fmc = false;
}

#endif

