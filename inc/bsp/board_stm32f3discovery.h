#ifndef _BOARD_STM32F3DISCOVERY
#define _BOARD_STM32F3DISCOVERY

// definition of resoures on atu first STM32F303 discovery board
// headers must be included manualy in C/CPP file

// default LEDS is E8:E15
#define BOARD_N_LEDS 8

#define BOARD_DEFINE_LEDS PinsOut leds( GPIOE, 8, BOARD_N_LEDS );

#endif
