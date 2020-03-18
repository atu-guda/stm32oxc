#include <oxc_base.h>
#include <board_sdram.h>

static bool bsp_allocated_fmc = false;

void* malloc_fmc( size_t sz )
{
  if( bsp_allocated_axi ) {
    bsp_allocated_fmc = true;
    return (void*)(SDRAM_BANK_ADDR);
  }
  return nullptr;
}

void  free_fmc( void* ptr )
{
  if( ptr != (void*)(SDRAM_BANK_ADDR) ) {
    return;
  }
  bsp_allocated_fmc = false;
}


