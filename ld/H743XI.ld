_FLASH_SIZE  = 2048K ;
_RAM_SIZE    =  512K ;
_CCMRAM_SIZE =    0K ;

/* non-common memory locations */
MEMORY
{
  FLASH  (rx)   : ORIGIN = 0x08000000, LENGTH = _FLASH_SIZE
  DTCMRAM(xrw)  : ORIGIN = 0x20000000, LENGTH = 128K   /* DTCMRAM */
  RAM    (xrw)  : ORIGIN = 0x24000000, LENGTH = _RAM_SIZE
  RAM_D2 (xrw)  : ORIGIN = 0x30000000, LENGTH = 288K
  RAM_D3 (xrw)  : ORIGIN = 0x38000000, LENGTH = 64K
  ITCMRAM (xrw) : ORIGIN = 0x00000000, LENGTH = 64K
  CCMRAM (rw)   : ORIGIN = 0x10000000, LENGTH = _CCMRAM_SIZE
}

/* INCLUDE "stm32_common.ld" */
INCLUDE "stm32_common_base.ld"

