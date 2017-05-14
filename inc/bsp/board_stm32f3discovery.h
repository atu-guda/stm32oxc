#ifndef _BOARD_STM32F3DISCOVERY
#define _BOARD_STM32F3DISCOVERY

// definition of resoures on atu first STM32F303 discovery board
// headers must be included manualy in C/CPP file

#define def_stksz 256

// default LEDS is E8:E15
#define BOARD_N_LEDS 8
#define BOARD_LEDS_GPIO GPIOE
#define BOARD_LEDS_GPIO_ON __GPIOE_CLK_ENABLE()
#define BOARD_LEDS_OFS  8
#define BOARD_LEDS_MASK 0xFF00
// unshifted
#define BOARD_LEDS_ALL  0x00FF

#define BOARD_DEFINE_LEDS PinsOut leds( BOARD_LEDS_GPIO, BOARD_LEDS_OFS, BOARD_N_LEDS );

// mini LEDS is E12:E15
#define BOARD_N_LEDS_MINI 4

#define BOARD_DEFINE_LEDS_MINI PinsOut leds( BOARD_LEDS_GPIO, 12, BOARD_N_LEDS_MINI );

#define LED_BSP_BLUE         1
#define LED_BSP_BLUE_0       1
#define LED_BSP_RED          2
#define LED_BSP_RED_0        2
#define LED_BSP_ORANGE       4
#define LED_BSP_ORANGE_0     4
#define LED_BSP_GREEN        8
#define LED_BSP_GREEN_0      8

#define LED_BSP_BLUE_1    0x10
#define LED_BSP_RED_1     0x20
#define LED_BSP_ORANGE_1  0x40
#define LED_BSP_GREEN_1   0x80

#define LED_BSP_IDLE      LED_BSP_ORANGE_1
#define LED_BSP_TX        LED_BSP_RED_0
#define LED_BSP_RX        LED_BSP_GREEN_0
#define LED_BSP_ERR       LED_BSP_RED_1

#define BOARD_BTN0_EXIST  1
#define BOARD_BTN0_GPIO   GPIOA
#define BOARD_BTN0_EN     __GPIOA_CLK_ENABLE();
#define BOARD_BTN0_N      0
#define BOARD_BTN0_BIT    ( 1 << BOARD_BTN0_N )
#define BOARD_BTN0_IRQ    EXTI0_IRQn
#define BOARD_BTN0_IRQHANDLER EXTI0_IRQHandler



#define TIM_EXA        TIM8
#define TIM_EXA_STR    "TIM8"
#define TIM_EXA_GPIO   GPIOC
#define TIM_EXA_PIN1   GPIO_PIN_6
#define TIM_EXA_PIN2   GPIO_PIN_7
#define TIM_EXA_PIN3   GPIO_PIN_8
#define TIM_EXA_PIN4   GPIO_PIN_9
#define TIM_EXA_PINS   ( TIM_EXA_PIN1 | TIM_EXA_PIN2 | TIM_EXA_PIN3 | TIM_EXA_PIN4 )
#define TIM_EXA_CLKEN  __GPIOC_CLK_ENABLE(); __TIM8_CLK_ENABLE();
#define TIM_EXA_CLKDIS __TIM8_CLK_DISABLE();
#define TIM_EXA_GPIOAF GPIO_AF4_TIM8
#define TIM_EXA_IRQ    TIM8_CC_IRQn
#define TIM_EXA_IRQHANDLER    TIM8_CC_IRQHandler

#define BOARD_UART_DEFAULT            USART2
#define BOARD_UART_DEFAULT_NAME       "USART2"
#define BOARD_UART_DEFAULT_GPIO       GPIOA
#define BOARD_UART_DEFAULT_GPIO_PINS  ( GPIO_PIN_2 | GPIO_PIN_3 )
#define BOARD_UART_DEFAULT_GPIO_AF    GPIO_AF7_USART2
#define BOARD_UART_DEFAULT_ENABLE     __USART2_CLK_ENABLE(); __GPIOA_CLK_ENABLE();
#define BOARD_UART_DEFAULT_DISABLE    __USART2_CLK_DISABLE();
#define BOARD_UART_DEFAULT_IRQ        USART2_IRQn
#define BOARD_UART_DEFAULT_IRQHANDLER USART2_IRQHandler

// TODO: add some other clocks
#define BOARD_I2C_DEFAULT               I2C1
#define BOARD_I2C_DEFAULT_NAME          "I2C1"
#define BOARD_I2C_DEFAULT_SPEED         100000;
#define BOARD_I2C_DEFAULT_TIMING_100    0x10808DD3; // 100 kHz over  72 MHz
#define BOARD_I2C_DEFAULT_TIMING_400    0x00702681; // 400 kHz over  72 MHz
#define BOARD_I2C_DEFAULT_TIMING_1M     0x00300D2E; // 1   MHz over  72 MHz
#define BOARD_I2C_DEFAULT_GPIO_SCL      GPIOB
#define BOARD_I2C_DEFAULT_GPIO_SDA      GPIOB
#define BOARD_I2C_DEFAULT_GPIO_PIN_SCL  GPIO_PIN_6
#define BOARD_I2C_DEFAULT_GPIO_PIN_SDA  GPIO_PIN_7
#define BOARD_I2C_DEFAULT_GPIO_AF       GPIO_AF4_I2C1
#define BOARD_I2C_DEFAULT_ENABLE        __I2C1_CLK_ENABLE(); __GPIOB_CLK_ENABLE();
#define BOARD_I2C_DEFAULT_DISABLE       __I2C1_CLK_DISABLE();
#define BOARD_I2C_DEFAULT_IRQ           I2C1_EV_IRQn
#define BOARD_I2C_DEFAULT_IRQHANDLER    I2C1_EV_IRQHandler


#define BOARD_SPI_DEFAULT               SPI2
#define BOARD_SPI_DEFAULT_NAME          "SPI2"
#define BOARD_SPI_DEFAULT_GPIO_SCK      GPIOB
#define BOARD_SPI_DEFAULT_GPIO_PIN_SCK  GPIO_PIN_13
#define BOARD_SPI_DEFAULT_GPIO_MISO     GPIOB
#define BOARD_SPI_DEFAULT_GPIO_PIN_MISO GPIO_PIN_14
#define BOARD_SPI_DEFAULT_GPIO_MOSI     GPIOB
#define BOARD_SPI_DEFAULT_GPIO_PIN_MOSI GPIO_PIN_15
#define BOARD_SPI_DEFAULT_GPIO_SNSS     GPIOB
// here number, as input to PinsOut
#define BOARD_SPI_DEFAULT_GPIO_PIN_SNSS 12
#define BOARD_SPI_DEFAULT_GPIO_EXT1     GPIOB
#define BOARD_SPI_DEFAULT_GPIO_PIN_EXT1 10
#define BOARD_SPI_DEFAULT_GPIO_EXT2     GPIOB
#define BOARD_SPI_DEFAULT_GPIO_PIN_EXT2 11
#define BOARD_SPI_DEFAULT_GPIO_AF       GPIO_AF5_SPI2
#define BOARD_SPI_DEFAULT_ENABLE        __SPI2_CLK_ENABLE(); __GPIOB_CLK_ENABLE();
#define BOARD_SPI_DEFAULT_DISABLE       __SPI2_CLK_DISABLE();
#define BOARD_SPI_DEFAULT_IRQ           SPI2_IRQn
#define BOARD_SPI_DEFAULT_IRQHANDLER    SPI2_IRQHandler


#define BOARD_MOTOR_DEFAULT_GPIO        GPIOD
#define BOARD_MOTOR_DEFAULT_PIN0        0

#define BOARD_1W_DEFAULT_GPIO           GPIOD
#define BOARD_1W_DEFAULT_PIN            0

#define BOARD_ADC_DEFAULT_GPIO0         GPIOC
#define BOARD_ADC_DEFAULT_PIN0          GPIO_PIN_0
#define BOARD_ADC_DEFAULT_CH0           ADC_CHANNEL_6
#define BOARD_ADC_DEFAULT_GPIO1         GPIOC
#define BOARD_ADC_DEFAULT_PIN1          GPIO_PIN_1
#define BOARD_ADC_DEFAULT_CH1           ADC_CHANNEL_7
#define BOARD_ADC_DEFAULT_GPIO2         GPIOC
#define BOARD_ADC_DEFAULT_PIN2          GPIO_PIN_2
#define BOARD_ADC_DEFAULT_CH2           ADC_CHANNEL_8
#define BOARD_ADC_DEFAULT_GPIO3         GPIOC
#define BOARD_ADC_DEFAULT_PIN3          GPIO_PIN_3
#define BOARD_ADC_DEFAULT_CH3           ADC_CHANNEL_9


#define BOARD_CONSOLE_DEFINES         UART_CONSOLE_DEFINES( USART2 );
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( USART2 );
#define BOARD_PROLOG                  STD_PROLOG_UART;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS( task_usart2_send );
#define BOARD_CREATE_STD_TASKS_UART   CREATE_STD_TASKS( task_usart2_send );
#define BOARD_POST_INIT_BLINK         delay_ms( PROLOG_LED_TIME ); leds.write( 0x01 ); delay_ms( PROLOG_LED_TIME );

#endif
