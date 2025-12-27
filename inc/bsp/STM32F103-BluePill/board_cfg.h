#ifndef _BOARD_STM32F103C8_SMALL
#define _BOARD_STM32F103C8_SMALL

#define _BOARD_CFG_DEFINED

// definition of resoures for STM32F103 small board: BluePill
// included from oxc_base.h, postactions - oxc_post_board_cfg.h

#define MC_FLASH_SIZE 65536
#define MC_RAM_SIZE   20480
#define MC_RAM1_SIZE  20480
#define def_stksz       256

#define DELAY_APPROX_COEFF  7040


#ifdef NEED_LEDS_EXTRA
  #define BOARD_LEDS_START PB12
  #define BOARD_N_LEDS 4
#else
  #define BOARD_LEDS_START PC14
  #define BOARD_N_LEDS 1
#endif


#define LED_BSP_BLUE         8_mask
#define LED_BSP_BLUE_0       8_mask
#define LED_BSP_RED          1_mask
#define LED_BSP_RED_0        1_mask
#define LED_BSP_ORANGE       2_mask
#define LED_BSP_ORANGE_0     2_mask
#define LED_BSP_GREEN        4_mask
#define LED_BSP_GREEN_0      4_mask

#define LED_BSP_IDLE      LED_BSP_ORANGE
#define LED_BSP_TX        LED_BSP_RED
#define LED_BSP_RX        LED_BSP_GREEN
#define LED_BSP_ERR       LED_BSP_RED

// extra button
#define BOARD_BTN0_GPIOX  A
#define BOARD_BTN0_N      0
#define BOARD_BTN0_ACTIVE_DOWN 0

// extra button
#define BOARD_BTN1_GPIOX  A
#define BOARD_BTN1_N      1
#define BOARD_BTN1_ACTIVE_DOWN 0


#define TIM_EXA              TIM2
#define TIM_EXA_PIN1         PA0
#define TIM_EXA_PIN2         PA1
#define TIM_EXA_PIN3         PA2
#define TIM_EXA_PIN4         PA3
#define TIM_EXA_GPIOAF         1
#define   TIM_EXA_PINS       { TIM_EXA_PIN1, TIM_EXA_PIN2, TIM_EXA_PIN3, TIM_EXA_PIN4 }
#define TIM_EXA_CLKEN        __TIM2_CLK_ENABLE();
#define TIM_EXA_CLKDIS       __TIM2_CLK_DISABLE();
#define TIM_EXA_IRQ          TIM2_IRQn

// #define SD_EXA_CK        PB2
// #define SD_EXA_D0        PC8
// #define SD_EXA_CMD       PD2
// #define SD_EXA_GPIOAF    GPIO_AF12_SDIO
// #define SD_EXA_CLKEN     __HAL_RCC_SDIO_CLK_ENABLE();
// #define SD_EXA_CLKDIS    __HAL_RCC_SDIO_CLK_DISABLE();

#ifndef BOARD_UART_DEFAULT
#define BOARD_UART_DEFAULT            USART1
#define BOARD_UART_DEFAULT_TX         PA9
#define BOARD_UART_DEFAULT_RX         PA10
#define BOARD_UART_DEFAULT_GPIO_AF    1
#define BOARD_UART_DEFAULT_ENABLE     __USART1_CLK_ENABLE();
#define BOARD_UART_DEFAULT_DISABLE    __USART1_CLK_DISABLE();
#define BOARD_UART_DEFAULT_IRQ        USART1_IRQn
#define BOARD_UART_DEFAULT_IRQHANDLER USART1_IRQHandler
#endif

#ifndef BOARD_I2C_DEFAULT
#define BOARD_I2C_DEFAULT               I2C1
#define BOARD_I2C_DEFAULT_SPEED         100000
#define BOARD_I2C_DEFAULT_SCL           PB6
#define BOARD_I2C_DEFAULT_SDA           PB7
#define BOARD_I2C_DEFAULT_GPIO_AF       1
#define BOARD_I2C_DEFAULT_ENABLE        __I2C1_CLK_ENABLE();
// 100 kHz over  72 MHz
#define BOARD_I2C_DEFAULT_TIMING_100    0x10808DD3
 // 400 kHz over  72 MHz
#define BOARD_I2C_DEFAULT_TIMING_400    0x00702681
 // 1   MHz over  72 MHz
#define BOARD_I2C_DEFAULT_TIMING_1M     0x00300D2E
#define BOARD_I2C_DEFAULT_ENABLE        __I2C1_CLK_ENABLE();
#define BOARD_I2C_DEFAULT_DISABLE       __I2C1_CLK_DISABLE();
#define BOARD_I2C_DEFAULT_IRQ           I2C1_EV_IRQn
#define BOARD_I2C_DEFAULT_IRQHANDLER    I2C1_EV_IRQHandler
#endif

#define BOARD_IN0                       PB0
#define BOARD_IN1                       P1
#define BOARD_IN2                       PB10


#ifndef BOARD_SPI_DEFAULT
#define BOARD_SPI_DEFAULT               SPI1
#define BOARD_SPI_DEFAULT_GPIO_SCK      PA5
#define BOARD_SPI_DEFAULT_GPIO_MISO     PA6
#define BOARD_SPI_DEFAULT_GPIO_MOSI     PA7
#define BOARD_SPI_DEFAULT_GPIO_SNSS     PA4
#define BOARD_SPI_DEFAULT_EXT1          PA2
#define BOARD_SPI_DEFAULT_EXT2          P3
#define BOARD_SPI_DEFAULT_GPIO_AF       1
#define BOARD_SPI_DEFAULT_ENABLE        __SPI1_CLK_ENABLE();
#define BOARD_SPI_DEFAULT_DISABLE       __SPI1_CLK_DISABLE();
#define BOARD_SPI_DEFAULT_IRQ           SPI1_IRQn
#define BOARD_SPI_DEFAULT_IRQHANDLER    SPI1_IRQHandler
#define BOARD_SPI_BAUDRATEPRESCALER_FAST SPI_BAUDRATEPRESCALER_2
#endif

// B12-B15
#define BOARD_MOTOR_DEFAULT             PB12
#define BOARD_MOTOR_DEFAULT_N           4
//

#define BOARD_1W_DEFAULT_GPIO           PB5

#define BOARD_ADC_MEM_MAX               (1024*4)
// #define BOARD_ADC_MEM_MAX_FMC           (1024*1024*8)
#define BOARD_ADC_COEFF                 3250000
#define BOARD_ADC_MALLOC                ::malloc
#define BOARD_ADC_FREE                  ::free
// #define BOARD_ADC_MALLOC_EXT            malloc_fmc
// #define BOARD_ADC_FREE_EXT              free_fmc


#define BOARD_CONSOLE_DEFINES         UART_CONSOLE_DEFINES( USART1 );
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( USART1 );
#define BOARD_PROLOG                  STD_PROLOG_UART;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS;
#define BOARD_POST_INIT_BLINK         delay_ms( PROLOG_LED_TIME ); leds.write( 0_mask ); delay_ms( PROLOG_LED_TIME );

#endif
