#ifndef _BOARD_STM32F334_NUCLEO64_H
#define _BOARD_STM32F334_NUCLEO64_H

#define _BOARD_CFG_DEFINED

// definition of resoures STM32F334R nucleo 64 board
// included from oxc_base.h, postactions - oxc_post_board_cfg.h

#define MC_FLASH_SIZE   65536
#define MC_RAM_SIZE     12288
#define MC_RAM1_SIZE    12288
#define MC_RAM2_SIZE     4096
#define def_stksz         256

#define DELAY_APPROX_COEFF  7040



// default: single LED on A5 - ignore for now

#ifdef NEED_LEDS_MINI
  #define BOARD_LEDS_START PA5
  #define BOARD_N_LEDS 1
#else
  #define BOARD_LEDS_START PC0
  #define BOARD_N_LEDS 4
#endif

// extra is ???
// #define BOARD_N_LEDS_EXTRA 8

// #define BOARD_DEFINE_LEDS_EXTRA PinsOut leds( BOARD_LEDS_GPIO, BOARD_LEDS_OFS, BOARD_N_LEDS_EXTRA );

#define LED_BSP_RED       1_mask
#define LED_BSP_RED_0     1_mask
#define LED_BSP_YELLOW    2_mask
#define LED_BSP_YELLOW_0  2_mask
#define LED_BSP_GREEN     4_mask
#define LED_BSP_GREEN_0   4_mask
#define LED_BSP_BLUE      8_mask
#define LED_BSP_BLUE_0    8_mask

#define LED_BSP_IDLE      LED_BSP_BLUE
#define LED_BSP_TX        LED_BSP_YELLOW
#define LED_BSP_RX        LED_BSP_GREEN
#define LED_BSP_ERR       LED_BSP_RED

// on-board blue button
#define BOARD_BTN0_GPIOX       C
#define BOARD_BTN0_N          13
#define BOARD_BTN0_ACTIVE_DOWN 0

// extra button
#define BOARD_BTN1_GPIOX       B
#define BOARD_BTN1_N           0
#define BOARD_BTN1_ACTIVE_DOWN 0



#ifndef   TIM_EXA
#define   TIM_EXA               TIM1
#define   TIM_EXA_PIN1          PA8
#define   TIM_EXA_PIN2          PA9
#define   TIM_EXA_PIN3          PA10
#define   TIM_EXA_PIN4          PA11
#define   TIM_EXA_GPIOAF        GPIO_AF6_TIM1
#define   TIM_EXA_PINS          { TIM_EXA_PIN1, TIM_EXA_PIN2, TIM_EXA_PIN3, TIM_EXA_PIN4 }
#define   TIM_EXA_PIN_EXT       PA11
#define   TIM_EXA_GPIOAF_EXT    GPIO_AF11_TIM1
#define   TIM_EXA_CLKEN         __TIM1_CLK_ENABLE();
#define   TIM_EXA_CLKDIS        __TIM1_CLK_DISABLE();
#define   TIM_EXA_IRQ           TIM1_CC_IRQn
#define   TIM_EXA_IRQHANDLER    TIM1_CC_IRQHandler
#endif

#ifndef   TIM_IN_EXA
#define   TIM_IN_EXA            TIM2
#define   TIM_IN_EXA_PIN1       PA0
#define   TIM_IN_EXA_PIN2       PA1
#define   TIM_IN_EXA_PIN3       PA2
#define   TIM_IN_EXA_PIN4       PA3
#define   TIM_IN_EXA_GPIOAF     GPIO_AF1_TIM2
#define   TIM_IN_EXA_CLKEN      __TIM2_CLK_ENABLE();
#define   TIM_IN_EXA_CLKDIS     __TIM2_CLK_DISABLE();
#define   TIM_IN_EXA_IRQ        TIM2_IRQn
#define   TIM_IN_EXA_IRQHANDLER TIM2_IRQHandler
#endif

// SDIO
// #define SD_EXA_CK        PB2
// #define SD_EXA_D0        PC8
// #define SD_EXA_CMD       PD2
// #define SD_EXA_GPIOAF    GPIO_AF12_SDIO
// #define SD_EXA_CLKEN     __HAL_RCC_SDIO_CLK_ENABLE();
// #define SD_EXA_CLKDIS    __HAL_RCC_SDIO_CLK_DISABLE();

#ifndef BOARD_UART_DEFAULT
#define BOARD_UART_DEFAULT            USART2
#define BOARD_UART_DEFAULT_TX         PA2
#define BOARD_UART_DEFAULT_RX         PA3
#define BOARD_UART_DEFAULT_GPIO_AF    GPIO_AF7_USART2
#define BOARD_UART_DEFAULT_ENABLE     __USART2_CLK_ENABLE();
#define BOARD_UART_DEFAULT_DISABLE    __USART2_CLK_DISABLE();
#define BOARD_UART_DEFAULT_IRQ        USART2_IRQn
#define BOARD_UART_DEFAULT_IRQHANDLER USART2_IRQHandler
#endif

#ifndef BOARD_I2C_DEFAULT
#define BOARD_I2C_DEFAULT               I2C1
#define BOARD_I2C_DEFAULT_SPEED         100000
#define BOARD_I2C_DEFAULT_SCL           PB8
#define BOARD_I2C_DEFAULT_SDA           PB9
#define BOARD_I2C_DEFAULT_GPIO_AF       GPIO_AF4_I2C1
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

#define BOARD_IN0                       PC10
#define BOARD_IN1                       PC11
#define BOARD_IN2                       PC12


#ifndef BOARD_SPI_DEFAULT
#define BOARD_SPI_DEFAULT               SPI1
#define BOARD_SPI_DEFAULT_PIN_SCK       PA5
#define BOARD_SPI_DEFAULT_PIN_MISO      PA6
#define BOARD_SPI_DEFAULT_PIN_MOSI      PA7
#define BOARD_SPI_DEFAULT_PIN_SNSS      PA8
#define BOARD_SPI_DEFAULT_PIN_EXT1      PA9
#define BOARD_SPI_DEFAULT_PIN_EXT2      PA10
#define BOARD_SPI_DEFAULT_GPIO_AF       GPIO_AF5_SPI1
#define BOARD_SPI_DEFAULT_ENABLE        __SPI1_CLK_ENABLE();
#define BOARD_SPI_DEFAULT_DISABLE       __SPI1_CLK_DISABLE();
#define BOARD_SPI_DEFAULT_IRQ           SPI1_IRQn
#define BOARD_SPI_DEFAULT_IRQHANDLER    SPI1_IRQHandler
#define BOARD_SPI_BAUDRATEPRESCALER_FAST SPI_BAUDRATEPRESCALER_2
#endif


// A8-A11
#define BOARD_MOTOR_DEFAULT             PA8
#define BOARD_MOTOR_DEFAULT_N           4

#define BOARD_1W_DEFAULT_PIN            PC4

// A0, A1, A6, A7 (0,1,5,7)
#define BOARD_ADC_DEFAULT_DEV           ADC1
#define BOARD_ADC_DEFAULT_EN            __HAL_RCC_ADC1_CLK_ENABLE();
#define BOARD_ADC_DEFAULT_DIS           __HAL_RCC_ADC1_CLK_DISABLE();
#define BOARD_ADC_DEFAULT_PIN0          PA0
#define BOARD_ADC_DEFAULT_CH0           ADC_CHANNEL_0
#define BOARD_ADC_DEFAULT_PIN1          PA1
#define BOARD_ADC_DEFAULT_CH1           ADC_CHANNEL_1
#define BOARD_ADC_DEFAULT_PIN2          PA6
#define BOARD_ADC_DEFAULT_CH2           ADC_CHANNEL_6
#define BOARD_ADC_DEFAULT_PIN3          PA7
#define BOARD_ADC_DEFAULT_CH3           ADC_CHANNEL_7
#define BOARD_ADC_MEM_MAX               (1024*4)
// #define BOARD_ADC_MEM_MAX_FMC           (1024*1024*8)
#define BOARD_ADC_COEFF                 3250000
#define BOARD_ADC_MALLOC                ::malloc
#define BOARD_ADC_FREE                  ::free
// #define BOARD_ADC_MALLOC_EXT            malloc_fmc
// #define BOARD_ADC_FREE_EXT              free_fmc


#define HX711_EXA_SCK_PIN  PC10
#define HX711_EXA_DAT_PIN  PC11

#define BOARD_CONSOLE_DEFINES         UART_CONSOLE_DEFINES( USART2 );
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( USART2 );
#define BOARD_PROLOG                  STD_PROLOG_UART;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS;
#define BOARD_POST_INIT_BLINK         delay_ms( PROLOG_LED_TIME ); leds.write( 0_mask ); delay_ms( PROLOG_LED_TIME );

#endif
