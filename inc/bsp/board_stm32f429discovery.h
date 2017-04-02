#ifndef _BOARD_STM32F429DISCOVERY_H
#define _BOARD_STM32F429DISCOVERY_H

// definition of resoures on atu first STM32F429I discovery board
// headers must be included manualy in C/CPP file

// Free pins:
//   A5, B4, B7, C3, C8, C11-13-?15, D2, D4, D5, D7, E1-E6, G1, G2, G9
// Free devs:
//   SPI3( B3, C11, C12, [-] )
//   SPI4( E2,  E5,  E6,  E4 )
//   TIM2(ch1,ch2)( A5,  B3 )
//   UART5
// Used internally, but can be used soveware:
//   USART1->STLINK.VCP(A9,A10)

// default LEDS is G13 (Green), G14 (Red)
#define BOARD_N_LEDS 2
#define BOARD_LEDS_GPIO GPIOG
#define BOARD_LEDS_GPIO_ON __GPIOG_CLK_ENABLE()
#define BOARD_LEDS_OFS  13
#define BOARD_LEDS_MASK 0x6000
// unshifted
#define BOARD_LEDS_ALL  0x03

#define BOARD_DEFINE_LEDS PinsOut leds( BOARD_LEDS_GPIO, BOARD_LEDS_OFS, BOARD_N_LEDS );

#define LED_BSP_GREEN     1
#define LED_BSP_GREEN_0   1
#define LED_BSP_RED       2
#define LED_BSP_RED_0     2

#define LED_BSP_IDLE      LED_BSP_GREEN
#define LED_BSP_TX        LED_BSP_RED
#define LED_BSP_RX        LED_BSP_GREEN
#define LED_BSP_ERR       LED_BSP_RED


#endif
