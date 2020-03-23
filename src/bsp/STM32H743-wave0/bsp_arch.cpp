#include <oxc_base.h>

#ifdef USE_OXC_SDRAM
#include <board_sdram.h>
#endif

static bool bsp_allocated_axi = false;

void* malloc_axi( size_t sz )
{
  if( bsp_allocated_axi || sz > (512*1024) ) {
    return nullptr;
  }
  bsp_allocated_axi = true;
  return (void*)(0x24000000);
}

void  free_axi( void* ptr )
{
  if( ptr != (void*)(0x24000000) ) {
    return;
  }
  bsp_allocated_axi = false;
}

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

