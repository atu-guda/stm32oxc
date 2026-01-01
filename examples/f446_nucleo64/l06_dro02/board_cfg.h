#ifndef _BOARD_STM32F446_NUCLEO64_H
#define _BOARD_STM32F446_NUCLEO64_H

#define _BOARD_CFG_DEFINED

// not so special config
// included from oxc_base.h, postactions - oxc_post_board_cfg.h

#define MC_FLASH_SIZE 524288
#define MC_RAM_SIZE   131072
#define MC_RAM1_SIZE  131072
#define def_stksz        512

#define DELAY_APPROX_COEFF  5010


// default: single LED on A5 - ignore for now

#ifdef NEED_LEDS_MINI
  #define BOARD_LEDS_START PA5
  #define BOARD_N_LEDS 1
#else
  #define BOARD_LEDS_START PC0
  #define BOARD_N_LEDS 4
#endif


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
//#define BOARD_BTN1_GPIOX       B
//#define BOARD_BTN1_N           0
//#define BOARD_BTN1_ACTIVE_DOWN 0


#define   TIM_EXA              TIM1
#define   TIM_EXA_PIN1         PA8
#define   TIM_EXA_PIN2         PA9
#define   TIM_EXA_PIN3         PA10
#define   TIM_EXA_PIN4         PA11
#define   TIM_EXA_GPIOAF       GPIO_AF1_TIM1
#define   TIM_EXA_PINS         { TIM_EXA_PIN1, TIM_EXA_PIN2, TIM_EXA_PIN3, TIM_EXA_PIN4 }
//#define TIM_EXA_PIN_EXT PA11
//#define TIM_EXA_GPIOAF_EXT GPIO_AF11_TIM1
#define   TIM_EXA_CLKEN        __TIM1_CLK_ENABLE();
#define   TIM_EXA_CLKDIS       __TIM1_CLK_DISABLE();
#define   TIM_EXA_IRQ          TIM1_CC_IRQn
#define   TIM_EXA_IRQHANDLER   TIM1_CC_IRQHandler

#define SD_EXA_CK        PB2
#define SD_EXA_D0        PC8
#define SD_EXA_CMD       PD2
#define SD_EXA_GPIOAF    GPIO_AF12_SDIO
#define SD_EXA_CLKEN     __HAL_RCC_SDIO_CLK_ENABLE();
#define SD_EXA_CLKDIS    __HAL_RCC_SDIO_CLK_DISABLE();

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
#define BOARD_I2C_DEFAULT_ENABLE        __I2C1_CLK_ENABLE();
#define BOARD_I2C_DEFAULT_DISABLE       __I2C1_CLK_DISABLE();
#define BOARD_I2C_DEFAULT_IRQ           I2C1_EV_IRQn
#define BOARD_I2C_DEFAULT_IRQHANDLER    I2C1_EV_IRQHandler
#endif

#define BOARD_IN0                       PC10
#define BOARD_IN1                       PC11
#define BOARD_IN2                       PC12


#ifndef BOARD_SPI_DEFAULT
#define BOARD_SPI_DEFAULT               SPI2
#define BOARD_SPI_DEFAULT_PIN_SCK       PB13
#define BOARD_SPI_DEFAULT_PIN_MISO      PB14
#define BOARD_SPI_DEFAULT_PIN_MOSI      PB15
#define BOARD_SPI_DEFAULT_PIN_SNSS      PB1
#define BOARD_SPI_DEFAULT_PIN_EXT1      PB4
#define BOARD_SPI_DEFAULT_PIN_EXT2      PB5
#define BOARD_SPI_DEFAULT_GPIO_AF       GPIO_AF5_SPI2
#define BOARD_SPI_DEFAULT_ENABLE        __SPI2_CLK_ENABLE();
#define BOARD_SPI_DEFAULT_DISABLE       __SPI2_CLK_DISABLE();
#define BOARD_SPI_DEFAULT_IRQ           SPI2_IRQn
#define BOARD_SPI_DEFAULT_IRQHANDLER    SPI2_IRQHandler
#define BOARD_SPI_BAUDRATEPRESCALER_FAST SPI_BAUDRATEPRESCALER_2
#endif


// A8-A11
#define BOARD_MOTOR_DEFAULT_PIN0        PA8
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
#define BOARD_ADC_MEM_MAX               (1024*96)
// #define BOARD_ADC_MEM_MAX_FMC           (1024*1024*8)
#define BOARD_ADC_COEFF                 3250000
#define BOARD_ADC_MALLOC                ::malloc
#define BOARD_ADC_FREE                  ::free
// #define BOARD_ADC_MALLOC_EXT            malloc_fmc
// #define BOARD_ADC_FREE_EXT              free_fmc

#define BOARD_ADC_DEFAULT_CLOCK         ADC_CLOCK_SYNC_PCLK_DIV4
#define BOARD_ADC_DEFAULT_SAMPL_LARGE   ADC_SAMPLETIME_480CYCLES
#define BOARD_ADC_DEFAULT_RESOLUTION    ADC_RESOLUTION_12B
#define BOARD_ADC_DEFAULT_BITS          12
#define BOARD_ADC_DEFAULT_MAX           0x0FFF
#define BOARD_ADC_IRQ                   ADC_IRQn
#define BOARD_ADC_IRQHANDLER            ADC_IRQHandler
#define BOARD_ADC_DMA_INSTANCE          DMA2_Stream0
#define BOARD_ADC_DMA_CHANNEL           DMA_CHANNEL_0
// #define BOARD_ADC_DMA_REQUEST           DMA_REQUEST_ADC1
#define BOARD_ADC_DMA_DEFAULT_EN        __HAL_RCC_DMA2_CLK_ENABLE();
#define BOARD_ADC_DMA_DEFAULT_DIS       __HAL_RCC_DMA2_CLK_DISABLE();
#define BOARD_ADC_DMA_IRQ               DMA2_Stream0_IRQn
#define BOARD_ADC_DMA_IRQHANDLER        DMA2_Stream0_IRQHandler
#define BOARD_ADC_DEFAULT_TRIG          ADC_EXTERNALTRIG2_T2_TRGO
#define BOARD_ADC_DEFAULT_DBLBUF        ((uint32_t)DMA_SxCR_DBM)

//void* malloc_fmc( size_t sz ); // only all FMC memory for now
//void  free_fmc( void* ptr );

#define HX711_EXA_SCK_PIN  PC10
#define HX711_EXA_DAT_PIN  PC11

// 0 = DEVICE_FS, 1 = DEVICE_HS
#define BOARD_USB_DEFAULT_TYPE       0
#define BOARD_USB_DEFAULT_INSTANCE   USB_OTG_FS
#define BOARD_USB_DEFAULT_GPIO       GpioA
#define BOARD_USB_DEFAULT_DPDM_PINS  PinMask( GPIO_PIN_11 | GPIO_PIN_12 )
#define BOARD_USB_DEFAULT_VBUS_PIN   9_mask
#define BOARD_USB_DEFAULT_ID_PIN     10_mask
#define BOARD_USB_DEFAULT_GPIO_AF    GPIO_AF10_OTG_FS
#define BOARD_USB_DEFAULT_ENABLE     __GPIOA_CLK_ENABLE(); __HAL_RCC_USB_OTG_FS_CLK_ENABLE(); __HAL_RCC_SYSCFG_CLK_ENABLE();
#define BOARD_USB_DEFAULT_DISABLE    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
#define BOARD_USB_DEFAULT_IRQ        OTG_FS_IRQn
#define BOARD_USB_DEFAULT_IRQHANDLER OTG_FS_IRQHandler
#define BOARD_USB_DEFAULT_IRQ_PRTY   14

#define BOARD_CONSOLE_DEFINES         UART_CONSOLE_DEFINES( USART2 );
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( USART2 );
#define BOARD_PROLOG                  STD_PROLOG_UART;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS;
#define BOARD_POST_INIT_BLINK         delay_ms( PROLOG_LED_TIME ); leds.write( 0_mask ); delay_ms( PROLOG_LED_TIME );

#endif
