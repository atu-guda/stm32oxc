#ifndef _BOARD_STM32F4DISCOVERY_H
#define _BOARD_STM32F4DISCOVERY_H

#define def_stksz 512

// definition of resoures on atu first STM32F407GE discovery
// headers must be included manualy in C/CPP file
//
// Free pins:
//  A1,A2,A3,A8,A15, B0,B1,B2,B4,B5,B7,B8,B11-B15, C1,C2,C4-C6,C8,C9, C11, C13-C15?
//  D0-D3, D6-D11, E3-E15,

// default LEDS is D12:D15
#define BOARD_N_LEDS 4
#define BOARD_LEDS_GPIO GPIOD
#define BOARD_LEDS_GPIO_ON __GPIOD_CLK_ENABLE()
#define BOARD_LEDS_OFS  12
#define BOARD_LEDS_MASK 0xF000
// unshifted
#define BOARD_LEDS_ALL  0x0F

#define BOARD_DEFINE_LEDS PinsOut leds( GPIOD, BOARD_LEDS_OFS, BOARD_N_LEDS );

#define LED_BSP_GREEN     1
#define LED_BSP_GREEN_0   1
#define LED_BSP_ORANGE    2
#define LED_BSP_ORANGE_0  2
#define LED_BSP_RED       4
#define LED_BSP_RED_0     4
#define LED_BSP_BLUE      8
#define LED_BSP_BLUE_0    8
//
#define LED_BSP_IDLE      LED_BSP_BLUE
#define LED_BSP_TX        LED_BSP_RED
#define LED_BSP_RX        LED_BSP_GREEN
#define LED_BSP_ERR       LED_BSP_ORANGE


#endif
