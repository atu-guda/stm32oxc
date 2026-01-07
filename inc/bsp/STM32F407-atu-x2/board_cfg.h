#ifndef _BOARD_STM32F407_ATU_X2_H
#define _BOARD_STM32F407_ATU_X2_H

#define _BOARD_CFG_DEFINED

// definition of resoures on atu first STM32F407VE based board (named X2)
// included from oxc_base.h, postactions - oxc_post_board_cfg.h


#define MC_FLASH_SIZE  524288
#define MC_RAM_SIZE    131072
#define MC_RAM1_SIZE   131072
#define def_stksz         512

#define DELAY_APPROX_COEFF  5010


// default LEDS is C0:C3

#ifdef NEED_LEDS_MINI
  #define BOARD_LEDS_START PC0
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
#define BOARD_BTN0_GPIOX       A
#define BOARD_BTN0_N           0
#define BOARD_BTN0_ACTIVE_DOWN 0

// button 1
#define BOARD_BTN1_GPIOX       A
#define BOARD_BTN1_N           1
#define BOARD_BTN1_ACTIVE_DOWN 0



#ifndef   TIM_EXA
#define   TIM_EXA               TIM1
#define   TIM_EXA_PIN1          PE9
#define   TIM_EXA_PIN2          PE11
#define   TIM_EXA_PIN3          PE13
#define   TIM_EXA_PIN4          PE14
#define   TIM_EXA_GPIOAF        GPIO_AF1_TIM1
#define   TIM_EXA_PINS          { TIM_EXA_PIN1, TIM_EXA_PIN2, TIM_EXA_PIN3, TIM_EXA_PIN4 }
//#define TIM_EXA_PIN_EXT       PA11
//#define TIM_EXA_GPIOAF_EXT    GPIO_AF11_TIM1
#define   TIM_EXA_CLKEN         __TIM1_CLK_ENABLE();
#define   TIM_EXA_CLKDIS        __TIM1_CLK_DISABLE();
#define   TIM_EXA_IRQ           TIM1_CC_IRQn
#define   TIM_EXA_IRQHANDLER    TIM1_CC_IRQHandler
#endif

#ifndef   TIM_IN_EXA
#define   TIM_IN_EXA            TIM2
#define   TIM_IN_EXA_PIN1       PA15
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
#define SD_EXA_CK        PC12
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
#define BOARD_I2C_DEFAULT_SCL           PB6
#define BOARD_I2C_DEFAULT_SDA           PB7
#define BOARD_I2C_DEFAULT_GPIO_AF       GPIO_AF4_I2C1
#define BOARD_I2C_DEFAULT_ENABLE        __I2C1_CLK_ENABLE();
#define BOARD_I2C_DEFAULT_DISABLE       __I2C1_CLK_DISABLE();
#define BOARD_I2C_DEFAULT_IRQ           I2C1_EV_IRQn
#define BOARD_I2C_DEFAULT_IRQHANDLER    I2C1_EV_IRQHandler
#endif

#define BOARD_IN0                       PD8
#define BOARD_IN1                       PD9
#define BOARD_IN2                       PD10


#ifndef BOARD_SPI_DEFAULT
#define BOARD_SPI_DEFAULT               SPI1
#define BOARD_SPI_DEFAULT_PIN_SCK       PA5
#define BOARD_SPI_DEFAULT_PIN_MISO      PA6
#define BOARD_SPI_DEFAULT_PIN_MOSI      PA7
#define BOARD_SPI_DEFAULT_PIN_SNSS      PA4
#define BOARD_SPI_DEFAULT_PIN_EXT1      PE7
#define BOARD_SPI_DEFAULT_PIN_EXT2      PE8
#define BOARD_SPI_DEFAULT_GPIO_AF       GPIO_AF5_SPI1
#define BOARD_SPI_DEFAULT_ENABLE        __SPI1_CLK_ENABLE();
#define BOARD_SPI_DEFAULT_DISABLE       __SPI1_CLK_DISABLE();
#define BOARD_SPI_DEFAULT_IRQ           SPI1_IRQn
#define BOARD_SPI_DEFAULT_IRQHANDLER    SPI1_IRQHandler
#define BOARD_SPI_BAUDRATEPRESCALER_FAST SPI_BAUDRATEPRESCALER_2
#endif


// Motor: E0-E3
#define BOARD_MOTOR_DEFAULT_PIN0        PE0
#define BOARD_MOTOR_DEFAULT_N           4

#define BOARD_1W_DEFAULT_PIN            PE15

// ADC: B0, B1, C4, C5 (8,9,14,15)
#define BOARD_ADC_DEFAULT_DEV           ADC1
#define BOARD_ADC_DEFAULT_EN            __HAL_RCC_ADC1_CLK_ENABLE();
#define BOARD_ADC_DEFAULT_DIS           __HAL_RCC_ADC1_CLK_DISABLE();
#define BOARD_ADC_DEFAULT_PIN0          PB0
#define BOARD_ADC_DEFAULT_CH0           ADC_CHANNEL_8
#define BOARD_ADC_DEFAULT_PIN1          PB1
#define BOARD_ADC_DEFAULT_CH1           ADC_CHANNEL_9
#define BOARD_ADC_DEFAULT_PIN2          PC4
#define BOARD_ADC_DEFAULT_CH2           ADC_CHANNEL_14
#define BOARD_ADC_DEFAULT_PIN3          PC5
#define BOARD_ADC_DEFAULT_CH3           ADC_CHANNEL_15
#define BOARD_ADC_MEM_MAX               (1024*64)
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

#define HX711_EXA_SCK_PIN  PC4
#define HX711_EXA_DAT_PIN  PC5

// 0 = DEVICE_FS, 1 = DEVICE_HS
#define BOARD_USB_DEFAULT_TYPE       0
#define BOARD_USB_DEFAULT_INSTANCE   USB_OTG_FS
#define BOARD_USB_DEFAULT_GPIO       GpioA
#define BOARD_USB_DEFAULT_DPDM_PINS  PinMask( GPIO_PIN_11 | GPIO_PIN_12 )
#define BOARD_USB_DEFAULT_VBUS_PIN   PA9
#define BOARD_USB_DEFAULT_ID_PIN     PA10
#define BOARD_USB_DEFAULT_GPIO_AF    GPIO_AF10_OTG_FS
#define BOARD_USB_DEFAULT_ENABLE     __GPIOA_CLK_ENABLE(); __HAL_RCC_USB_OTG_FS_CLK_ENABLE(); __HAL_RCC_SYSCFG_CLK_ENABLE();
#define BOARD_USB_DEFAULT_DISABLE    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
#define BOARD_USB_DEFAULT_IRQ        OTG_FS_IRQn
#define BOARD_USB_DEFAULT_IRQHANDLER OTG_FS_IRQHandler
#define BOARD_USB_DEFAULT_IRQ_PRTY   14

#define BOARD_CONSOLE_DEFINES         USBCDC_CONSOLE_DEFINES;
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( USART2 );
#define BOARD_PROLOG                  STD_PROLOG_USBCDC;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS;
#define BOARD_POST_INIT_BLINK         delay_ms( PROLOG_LED_TIME ); leds.write( 0_mask ); delay_ms( PROLOG_LED_TIME );

#endif
