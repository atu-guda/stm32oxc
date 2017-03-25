#ifndef _BOARD_STM32F103C8_SMALL
#define _BOARD_STM32F103C8_SMALL

// definition of resoures on atu first STM32F103 small board
// headers must be included manualy in C/CPP file

// signle onboard LED is C13
#define BOARD_N_LEDS 1
#define BOARD_LEDS_GPIO GPIOC
#define BOARD_LEDS_OFS  13
#define BOARD_LEDS_MASK 0x6000

#define BOARD_DEFINE_LEDS PinsOut leds( BOARD_LEDS_GPIO, BOARD_LEDS_OFS, BOARD_N_LEDS );

// extra LEDS is B12:B15
#define BOARD_N_LEDS_EXTRA 4

#define BOARD_DEFINE_LEDS_EXTRA PinsOut leds( GPIOB, 12, BOARD_N_LEDS_EXTRA );

#define LED_BSP_BLUE         8
#define LED_BSP_BLUE_0       8
#define LED_BSP_RED          1
#define LED_BSP_RED_0        1
#define LED_BSP_ORANGE       2
#define LED_BSP_ORANGE_0     2
#define LED_BSP_GREEN        4
#define LED_BSP_GREEN_0      4

#define LED_BSP_IDLE      LED_BSP_ORANGE
#define LED_BSP_TX        LED_BSP_RED
#define LED_BSP_RX        LED_BSP_GREEN
#define LED_BSP_ERR       LED_BSP_RED

#endif
