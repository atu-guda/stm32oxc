#ifndef _BOARD_STM32F407_ATU_X2_H
#define _BOARD_STM32F407_ATU_X2_H

// definition of resoures on atu first STM32F407VE based board (named X2)
// headers must be included manualy in C/CPP file

// default LEDS is C0:C3
#define BOARD_N_LEDS 4

#define BOARD_DEFINE_LEDS PinsOut leds( GPIOC, 0, BOARD_N_LEDS );

#endif
