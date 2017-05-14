#ifndef _BOARD_STM32F429DISCOVERY_H
#define _BOARD_STM32F429DISCOVERY_H


// definition of resoures on STM32F429I discovery board.
// headers must be included manualy in C/CPP file

#define def_stksz 512

// Free pins:
//   A5, B4, B7, C3, C8, C11-13-?15, D2, D4, D5, D7, E1-E6, G1, G2, G9
// Free devs:
//   SPI3( B3, C11, C12, [-] )
//   SPI4( E2,  E5,  E6,  E4 )
//   TIM2(ch1,ch2)( A5,  B3 )
//   TIM9(ch1,ch2)( E5, E6 )
//   UART5
// Used internally, but can be used someware:
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

#define BOARD_BTN0_EXIST  1
#define BOARD_BTN0_GPIO   GPIOA
#define BOARD_BTN0_EN     __GPIOA_CLK_ENABLE();
#define BOARD_BTN0_N      0
#define BOARD_BTN0_BIT    ( 1 << BOARD_BTN0_N )
#define BOARD_BTN0_IRQ    EXTI0_IRQn
#define BOARD_BTN0_IRQHANDLER EXTI0_IRQHandler



#define TIM_EXA        TIM9
#define TIM_EXA_STR    "TIM9"
#define TIM_EXA_GPIO   GPIOE
#define TIM_EXA_PIN1   GPIO_PIN_5
#define TIM_EXA_PIN2   GPIO_PIN_6
// #define TIM_EXA_PIN3   0
// #define TIM_EXA_PIN4   0
#define TIM_EXA_PINS   ( TIM_EXA_PIN1 | TIM_EXA_PIN2  )
#define TIM_EXA_CLKEN  __GPIOE_CLK_ENABLE(); __TIM9_CLK_ENABLE();
#define TIM_EXA_CLKDIS __TIM9_CLK_DISABLE();
#define TIM_EXA_GPIOAF GPIO_AF3_TIM9
#define TIM_EXA_IRQ    TIM1_BRK_TIM9_IRQn
#define TIM_EXA_IRQHANDLER    TIM1_BRK_TIM9_IRQHandler

#define BOARD_UART_DEFAULT            USART1
#define BOARD_UART_DEFAULT_NAME       "USART1"
#define BOARD_UART_DEFAULT_GPIO       GPIOA
#define BOARD_UART_DEFAULT_GPIO_PINS  ( GPIO_PIN_9 | GPIO_PIN_10 )
#define BOARD_UART_DEFAULT_GPIO_AF    GPIO_AF7_USART1
#define BOARD_UART_DEFAULT_ENABLE     __USART1_CLK_ENABLE(); __GPIOA_CLK_ENABLE();
#define BOARD_UART_DEFAULT_DISABLE    __USART1_CLK_DISABLE();
#define BOARD_UART_DEFAULT_IRQ        USART1_IRQn
#define BOARD_UART_DEFAULT_IRQHANDLER USART1_IRQHandler

#define BOARD_I2C_DEFAULT               I2C3
#define BOARD_I2C_DEFAULT_NAME          "I2C3"
#define BOARD_I2C_DEFAULT_SPEED         100000;
#define BOARD_I2C_DEFAULT_GPIO_SCL      GPIOA
#define BOARD_I2C_DEFAULT_GPIO_SDA      GPIOC
#define BOARD_I2C_DEFAULT_GPIO_PIN_SCL  GPIO_PIN_8
#define BOARD_I2C_DEFAULT_GPIO_PIN_SDA  GPIO_PIN_9
#define BOARD_I2C_DEFAULT_GPIO_AF       GPIO_AF4_I2C1
#define BOARD_I2C_DEFAULT_ENABLE        __I2C3_CLK_ENABLE(); __GPIOA_CLK_ENABLE(); __GPIOC_CLK_ENABLE();
#define BOARD_I2C_DEFAULT_DISABLE       __I2C3_CLK_DISABLE();
#define BOARD_I2C_DEFAULT_IRQ           I2C3_EV_IRQn
#define BOARD_I2C_DEFAULT_IRQHANDLER    I2C3_EV_IRQHandler


#define BOARD_MOTOR_DEFAULT_GPIO        GPIOE
#define BOARD_MOTOR_DEFAULT_PIN0        1

#define BOARD_1W_DEFAULT_GPIO           GPIOG
#define BOARD_1W_DEFAULT_PIN            GPIO_PIN_9

#define BOARD_ADC_DEFAULT_GPIO0         GPIOA
#define BOARD_ADC_DEFAULT_PIN0          GPIO_PIN_5
#define BOARD_ADC_DEFAULT_CH0           ADC_CHANNEL_5
#define BOARD_ADC_DEFAULT_GPIO1         GPIOC
#define BOARD_ADC_DEFAULT_PIN1          GPIO_PIN_3
#define BOARD_ADC_DEFAULT_CH1           ADC_CHANNEL_13
#define BOARD_ADC_DEFAULT_GPIO2         GPIOA
#define BOARD_ADC_DEFAULT_PIN2          GPIO_PIN_5
#define BOARD_ADC_DEFAULT_CH2           ADC_CHANNEL_5
#define BOARD_ADC_DEFAULT_GPIO3         GPIOC
#define BOARD_ADC_DEFAULT_PIN3          GPIO_PIN_3
#define BOARD_ADC_DEFAULT_CH3           ADC_CHANNEL_13


#define BOARD_CONSOLE_DEFINES         UART_CONSOLE_DEFINES( USART1 );
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( USART1 );
#define BOARD_PROLOG                  STD_PROLOG_UART;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS( task_usart1_send );
#define BOARD_CREATE_STD_TASKS_UART   CREATE_STD_TASKS( task_usart1_send );
#define BOARD_POST_INIT_BLINK         delay_ms( PROLOG_LED_TIME ); leds.write( 0x01 ); delay_ms( PROLOG_LED_TIME );

#endif
