#ifndef _BOARD_STM32F103C8_SMALL
#define _BOARD_STM32F103C8_SMALL

// definition of resoures on atu first STM32F103 small board
// headers must be included manualy in C/CPP file

#define def_stksz 256

#ifndef NEED_LEDS_EXTRA

// signle onboard LED is C13
#define BOARD_N_LEDS 1
#define BOARD_LEDS_GPIO GPIOC
#define BOARD_LEDS_GPIO_ON __GPIOC_CLK_ENABLE()
#define BOARD_LEDS_OFS  13
#define BOARD_LEDS_MASK 0x6000
// unshifted
#define BOARD_LEDS_ALL  0x01

#define BOARD_DEFINE_LEDS PinsOut leds( BOARD_LEDS_GPIO, BOARD_LEDS_OFS, BOARD_N_LEDS );
#else
// #warning "extra leds"
#define BOARD_N_LEDS 4
#define BOARD_LEDS_GPIO GPIOB
#define BOARD_LEDS_GPIO_ON __GPIOB_CLK_ENABLE()
#define BOARD_LEDS_OFS  12
#define BOARD_LEDS_MASK 0xF000
// unshifted
#define BOARD_LEDS_ALL  0x0F
#endif

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

#define BOARD_BTN0_EXIST  1
#define BOARD_BTN0_GPIO   GPIOA
#define BOARD_BTN0_EN     __GPIOA_CLK_ENABLE();
#define BOARD_BTN0_N      0
#define BOARD_BTN0_BIT    ( 1 << BOARD_BTN0_N )
#define BOARD_BTN0_IRQ    EXTI0_IRQn
#define BOARD_BTN0_IRQHANDLER EXTI0_IRQHandler

// TODO: move to B6, B7 after tcouple project redesign
#define BOARD_UART_DEFAULT            USART1
#define BOARD_UART_DEFAULT_NAME       "USART1"
#define BOARD_UART_DEFAULT_GPIO       GPIOA
#define BOARD_UART_DEFAULT_GPIO_PINS  ( GPIO_PIN_9 | GPIO_PIN_10 )
#define BOARD_UART_DEFAULT_GPIO_TX    GPIO_PIN_9
#define BOARD_UART_DEFAULT_GPIO_RX    GPIO_PIN_10
// #define BOARD_UART_DEFAULT_GPIO_AF    GPIO_AF7_USART1
#define BOARD_UART_DEFAULT_ENABLE     __USART1_CLK_ENABLE(); __GPIOA_CLK_ENABLE();
#define BOARD_UART_DEFAULT_DISABLE    __USART1_CLK_DISABLE();
#define BOARD_UART_DEFAULT_IRQ        USART1_IRQn
#define BOARD_UART_DEFAULT_IRQHANDLER USART1_IRQHandler



#define BOARD_CONSOLE_DEFINES         UART_CONSOLE_DEFINES( USART1 );
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( USART1 );
#define BOARD_PROLOG                  STD_PROLOG_UART;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS( task_usart1_send );
#define BOARD_CREATE_STD_TASKS_UART   CREATE_STD_TASKS( task_usart1_send );
#define BOARD_POST_INIT_BLINK delay_ms( PROLOG_LED_TIME ); leds.write( 0x01 ); delay_ms( PROLOG_LED_TIME );

#endif
