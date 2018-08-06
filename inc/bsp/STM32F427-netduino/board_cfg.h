#ifndef _BOARD_STM32F427_NETDU_H
#define _BOARD_STM32F427_NETDU_H

#define def_stksz 512

// definition of resoures on STM32F427 netduino 3 board
// headers must be included manualy in C/CPP file

// not so default default LEDS is C0:C3 - external subboard
#define BOARD_N_LEDS 4
#define BOARD_LEDS_GPIO GPIOC
#define BOARD_LEDS_GPIO_ON __GPIOC_CLK_ENABLE()
#define BOARD_LEDS_OFS  0
#define BOARD_LEDS_MASK 0x000F
// unshifted
#define BOARD_LEDS_ALL  0x0F

#define BOARD_DEFINE_LEDS PinsOut leds( BOARD_LEDS_GPIO, BOARD_LEDS_OFS, BOARD_N_LEDS );

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

#define BOARD_BTN0_EXIST  1
#define BOARD_BTN0_GPIO   GPIOB
#define BOARD_BTN0_EN     __GPIOB_CLK_ENABLE();
#define BOARD_BTN0_N      5
#define BOARD_BTN0_BIT    ( 1 << BOARD_BTN0_N )
#define BOARD_BTN0_IRQ    EXTI9_5_IRQn
#define BOARD_BTN0_IRQHANDLER EXTI9_5_IRQHandler

#define BOARD_DEFINE_GO_LEDS \
  PinsOut led_go1( GPIOE,  9, 1 ); \
  PinsOut led_go2( GPIOE, 11, 1 ); \
  PinsOut led_go3( GPIOE, 14, 1 ); \

#define BOARD_DEFINE_GO_PWR \
  PinsOut pwr_go1( GPIOD,  7, 1 ); \
  PinsOut pwr_go2( GPIOD, 10, 1 ); \
  PinsOut pwr_go3( GPIOD, 12, 1 ); \

#define GO_IO_GPIO    GPIOD
#define GO_IO_CLKEN __GPIOD_CLK_ENABLE();
#define GO1_IO_PIN  13
#define GO2_IO_PIN  14
#define GO3_IO_PIN  15

#define GO_TXPULLUP_GPIO    GPIOE
#define GO_TXPULLUP_CLKEN __GPIOE_CLK_ENABLE();
#define GO1_TXPULLUP_PIN   3
#define GO2_TXPULLUP_PIN  10
#define GO3_TXPULLUP_PIN  12
#define BOARD_DEFINE_GO_TXPULLUP \
  PinsOut txpullup_go1( GO_TXPULLUP_GPIO, GO1_TXPULLUP_PIN, 1 ); \
  PinsOut txpullup_go2( GO_TXPULLUP_GPIO, GO2_TXPULLUP_PIN, 1 ); \
  PinsOut txpullup_g03( GO_TXPULLUP_GPIO, GO3_TXPULLUP_PIN, 1 ); \

#define GO_SPI SPI4
#define BOARD_DEFINE_GO1_CS   PinsOut spi_cs_go1( GPIOD, 0, 1 );
#define BOARD_DEFINE_GO2_CS   PinsOut spi_cs_go2( GPIOD, 1, 1 );
#define BOARD_DEFINE_GO3_CS   PinsOut spi_cs_go3( GPIOD, 2, 1 );

// #define TIM_EXA        TIM1
// #define TIM_EXA_STR    "TIM1"
// #define TIM_EXA_GPIO   GPIOE
// #define TIM_EXA_PIN1   GPIO_PIN_9
// #define TIM_EXA_PIN2   GPIO_PIN_11
// #define TIM_EXA_PIN3   GPIO_PIN_13
// #define TIM_EXA_PIN4   GPIO_PIN_14
// #define TIM_EXA_PINS   ( TIM_EXA_PIN1 | TIM_EXA_PIN2 | TIM_EXA_PIN3 | TIM_EXA_PIN4 )
// #define TIM_EXA_CLKEN  __GPIOE_CLK_ENABLE(); __TIM1_CLK_ENABLE();
// #define TIM_EXA_CLKDIS __TIM1_CLK_DISABLE();
// #define TIM_EXA_GPIOAF GPIO_AF1_TIM1
// #define TIM_EXA_IRQ    TIM1_BRK_TIM9_IRQn

// #define SD_EXA_CK_GPIO   GPIOC
// #define SD_EXA_CK_PIN    GPIO_PIN_12
// #define SD_EXA_D0_GPIO   GPIOC
// #define SD_EXA_D0_PIN    GPIO_PIN_8
// #define SD_EXA_CMD_GPIO  GPIOD
// #define SD_EXA_CMD_PIN   GPIO_PIN_2
// #define SD_EXA_CLKEN     __HAL_RCC_SDIO_CLK_ENABLE();  __HAL_RCC_GPIOC_CLK_ENABLE();  __HAL_RCC_GPIOD_CLK_ENABLE();
// #define SD_EXA_CLKDIS    __HAL_RCC_SDIO_CLK_DISABLE();
// #define SD_EXA_GPIOAF    GPIO_AF12_SDIO

#define BOARD_UART_DEFAULT            UART8
#define BOARD_UART_DEFAULT_NAME       "UART8"
#define BOARD_UART_DEFAULT_GPIO       GPIOE
#define BOARD_UART_DEFAULT_GPIO_PINS  ( GPIO_PIN_1 | GPIO_PIN_0 )
#define BOARD_UART_DEFAULT_GPIO_AF    GPIO_AF8_UART8
#define BOARD_UART_DEFAULT_ENABLE     __UART8_CLK_ENABLE(); __GPIOE_CLK_ENABLE();
#define BOARD_UART_DEFAULT_DISABLE    __UART8_CLK_DISABLE();
#define BOARD_UART_DEFAULT_IRQ        UART8_IRQn
#define BOARD_UART_DEFAULT_IRQHANDLER UART8_IRQHandler


#define BOARD_USB_DEFAULT_GPIO       GPIOA
#define BOARD_USB_DEFAULT_DPDM_PINS  ( GPIO_PIN_11 | GPIO_PIN_12 )
#define BOARD_USB_DEFAULT_VBUS_PIN   GPIO_PIN_9
#define BOARD_USB_DEFAULT_ID_PIN     GPIO_PIN_10
#define BOARD_USB_DEFAULT_GPIO_AF    GPIO_AF10_OTG_FS
#define BOARD_USB_DEFAULT_ENABLE     __GPIOA_CLK_ENABLE(); __HAL_RCC_USB_OTG_FS_CLK_ENABLE(); __HAL_RCC_SYSCFG_CLK_ENABLE();
#define BOARD_USB_DEFAULT_DISABLE    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
#define BOARD_USB_DEFAULT_IRQ        OTG_FS_IRQn
#define BOARD_USB_DEFAULT_IRQHANDLER OTG_FS_IRQHandler
#define BOARD_USB_DEFAULT_IRQ_PRTY   14

#define BOARD_CONSOLE_DEFINES         USBCDC_CONSOLE_DEFINES;
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( UART8 );
#define BOARD_PROLOG                  STD_PROLOG_USBCDC;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS( task_usbcdc_send );
#define BOARD_CREATE_STD_TASKS_UART   CREATE_STD_TASKS( task_usart2_send );
#define BOARD_POST_INIT_BLINK         delay_ms( PROLOG_LED_TIME ); leds.write( 0x01 ); delay_ms( PROLOG_LED_TIME );

#endif
