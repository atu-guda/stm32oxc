#ifndef _OXC_SDRAM_COMMON_H
#define _OXC_SDRAM_COMMON_H

#include <oxc_base.h>

#include <board_sdram.h>

extern SDRAM_HandleTypeDef hsdram_main;

int bsp_init_sdram();
int SDRAM_Initialization_Sequence( SDRAM_HandleTypeDef *hsdram );


#endif

// vim: path=.,/usr/share/stm32cube/inc
