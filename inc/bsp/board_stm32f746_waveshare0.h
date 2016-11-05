#ifndef _BOARD_STM32F746_WAVESHARE0_H
#define _BOARD_STM32F746_WAVESHARE0_H

// definition of resoures on STM32F746IGT WaveShare board
// headers must be included manualy in C/CPP file

// default LEDS is C0:C3
#define BOARD_N_LEDS 4
// extra is C0:C7
#define BOARD_N_LEDS_EXTRA 8

#define BOARD_DEFINE_LEDS PinsOut leds( GPIOC, 0, BOARD_N_LEDS );
#define BOARD_DEFINE_LEDS_EXTRA PinsOut leds( GPIOC, 0, BOARD_N_LEDS_EXTRA );

#define LED_BSP_RED       1
#define LED_BSP_RED_0     1
#define LED_BSP_YELLOW    2
#define LED_BSP_YELLOW_0  2
#define LED_BSP_GREEN     4
#define LED_BSP_GREEN_0   4
#define LED_BSP_BLUE      8
#define LED_BSP_BLUE_0    8

#define LED_BSP_IDLE      LED_BSP_BLUE
#define LED_BSP_TX        LED_BSP_RED
#define LED_BSP_RX        LED_BSP_GREEN
#define LED_BSP_ERR       LED_BSP_BLUE


#endif
