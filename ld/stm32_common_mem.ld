
MEMORY
{
  FLASH  (rx)   : ORIGIN = 0x08000000, LENGTH = _FLASH_SIZE
  RAM    (xrw)  : ORIGIN = 0x20000000, LENGTH = _RAM_SIZE
  CCMRAM (rw)   : ORIGIN = 0x10000000, LENGTH = _CCMRAM_SIZE
}

