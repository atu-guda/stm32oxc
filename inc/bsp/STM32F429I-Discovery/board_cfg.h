#ifndef _BOARD_STM32F429DISCOVERY_H
#define _BOARD_STM32F429DISCOVERY_H

#define _BOARD_CFG_DEFINED

// definition of resoures on STM32F429I discovery board.
// included from oxc_base.h, postactions - oxc_post_board_cfg.h


#define MC_FLASH_SIZE 2097152
#define MC_RAM_SIZE   196608
#define MC_RAM1_SIZE  196608
#define def_stksz        512

#define DELAY_APPROX_COEFF  5010

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

// on my board A1, A2 is not connected to MEMS, so may be used as ADC...

// default LEDS is G13 (Green), G14 (Red)
#ifdef NEED_LEDS_MINI
  #define BOARD_LEDS_START PG13
  #define BOARD_N_LEDS 1
#else
  #define BOARD_LEDS_START PG13
  #define BOARD_N_LEDS 2
#endif

#ifdef NEED_LEDS_EXTRA
  #define BOARD_DEFINE_LEDS_EXTRA PinsOut ledsx( PE1, 5 ); // E1-E6
#endif

#define LED_BSP_GREEN     1_mask
#define LED_BSP_GREEN_0   1_mask
#define LED_BSP_RED       2_mask
#define LED_BSP_RED_0     2_mask

#define LED_BSP_IDLE      LED_BSP_GREEN
#define LED_BSP_TX        LED_BSP_RED
#define LED_BSP_RX        LED_BSP_GREEN
#define LED_BSP_ERR       LED_BSP_RED

// on-board button
#define BOARD_BTN0_GPIOX       A
#define BOARD_BTN0_N           0
#define BOARD_BTN0_ACTIVE_DOWN 0

// extra button
#define BOARD_BTN1_GPIOX       A
#define BOARD_BTN1_N           1
#define BOARD_BTN1_ACTIVE_DOWN 0


#define   TIM_EXA              TIM9
#define   TIM_EXA_PIN1         PE5
#define   TIM_EXA_PIN2         PE6
// #define TIM_EXA_PIN3          0
// #define TIM_EXA_PIN4          0
#define   TIM_EXA_GPIOAF       GPIO_AF3_TIM9
#define   TIM_EXA_PINS       { TIM_EXA_PIN1, TIM_EXA_PIN2 }
//#define TIM_EXA_PIN_EXT        PA11
//#define TIM_EXA_GPIOAF_EXT     GPIO_AF11_TIM1
#define   TIM_EXA_CLKEN        __TIM9_CLK_ENABLE();
#define   TIM_EXA_CLKDIS       __TIM9_CLK_DISABLE();
#define   TIM_EXA_IRQ          TIM1_BRK_TIM9_IRQn
#define   TIM_EXA_IRQHANDLER   TIM1_BRK_TIM9_IRQHandler

// SDIO
#define SD_EXA_CK        PC12
#define SD_EXA_D0        PC8
#define SD_EXA_CMD       PD2
#define SD_EXA_GPIOAF    GPIO_AF12_SDIO
#define SD_EXA_CLKEN     __HAL_RCC_SDIO_CLK_ENABLE();
#define SD_EXA_CLKDIS    __HAL_RCC_SDIO_CLK_DISABLE();

#ifndef BOARD_UART_DEFAULT
#define BOARD_UART_DEFAULT            USART1
#define BOARD_UART_DEFAULT_TX         PA9
#define BOARD_UART_DEFAULT_RX         PA10
#define BOARD_UART_DEFAULT_GPIO_AF    GPIO_AF7_USART1
#define BOARD_UART_DEFAULT_ENABLE     __USART1_CLK_ENABLE(); __GPIOA_CLK_ENABLE();
#define BOARD_UART_DEFAULT_DISABLE    __USART1_CLK_DISABLE();
#define BOARD_UART_DEFAULT_IRQ        USART1_IRQn
#define BOARD_UART_DEFAULT_IRQHANDLER USART1_IRQHandler
#endif

#ifndef BOARD_I2C_DEFAULT
#define BOARD_I2C_DEFAULT               I2C3
#define BOARD_I2C_DEFAULT_SPEED         100000
#define BOARD_I2C_DEFAULT_SCL           PA8
#define BOARD_I2C_DEFAULT_SDA           PC9
#define BOARD_I2C_DEFAULT_GPIO_AF       GPIO_AF4_I2C3
#define BOARD_I2C_DEFAULT_ENABLE        __I2C3_CLK_ENABLE();
#define BOARD_I2C_DEFAULT_DISABLE       __I2C3_CLK_DISABLE();
#define BOARD_I2C_DEFAULT_IRQ           I2C3_EV_IRQn
#define BOARD_I2C_DEFAULT_IRQHANDLER    I2C3_EV_IRQHandler
#endif

#define BOARD_IN0                       PB4
#define BOARD_IN1                       PB7
#define BOARD_IN2                       PC11


#ifndef BOARD_SPI_DEFAULT
#define BOARD_SPI_DEFAULT               SPI5
#define BOARD_SPI_DEFAULT_PIN_SCK       PF7
#define BOARD_SPI_DEFAULT_PIN_MISO      PF8
#define BOARD_SPI_DEFAULT_PIN_MOSI      PF9
#define BOARD_SPI_DEFAULT_PIN_SNSS      PF6
#define BOARD_SPI_DEFAULT_PIN_EXT1      PF5
#define BOARD_SPI_DEFAULT_PIN_EXT2      PE6
#define BOARD_SPI_DEFAULT_GPIO_AF       GPIO_AF5_SPI5
#define BOARD_SPI_DEFAULT_ENABLE        __SPI5_CLK_ENABLE();
#define BOARD_SPI_DEFAULT_DISABLE       __SPI5_CLK_DISABLE();
#define BOARD_SPI_DEFAULT_IRQ           SPI1_IRQn
#define BOARD_SPI_DEFAULT_IRQHANDLER    SPI1_IRQHandler
#define BOARD_SPI_BAUDRATEPRESCALER_FAST SPI_BAUDRATEPRESCALER_2
#endif


// Motor: E0-E3
#define BOARD_MOTOR_DEFAULT_PIN0        PE0
#define BOARD_MOTOR_DEFAULT_N           4

#define BOARD_1W_DEFAULT_PIN            PG9

// ADC: A1, A2, A5, C3 (1,2,5,13)
#define BOARD_ADC_DEFAULT_DEV           ADC1
#define BOARD_ADC_DEFAULT_EN            __HAL_RCC_ADC1_CLK_ENABLE();
#define BOARD_ADC_DEFAULT_DIS           __HAL_RCC_ADC1_CLK_DISABLE();
#define BOARD_ADC_DEFAULT_PIN0          PA1
#define BOARD_ADC_DEFAULT_CH0           ADC_CHANNEL_1
#define BOARD_ADC_DEFAULT_PIN1          PA2
#define BOARD_ADC_DEFAULT_CH1           ADC_CHANNEL_2
#define BOARD_ADC_DEFAULT_PIN2          PA5
#define BOARD_ADC_DEFAULT_CH2           ADC_CHANNEL_5
#define BOARD_ADC_DEFAULT_PIN3          PC3
#define BOARD_ADC_DEFAULT_CH3           ADC_CHANNEL_13
#define BOARD_ADC_MEM_MAX               (1024*136)
#define BOARD_ADC_MEM_MAX_FMC           (1024*1024*8-10*1024)
#define BOARD_ADC_COEFF                 2937000
#define BOARD_ADC_MALLOC                ::malloc
#define BOARD_ADC_FREE                  ::free
#define BOARD_ADC_MALLOC_EXT            malloc_fmc
#define BOARD_ADC_FREE_EXT              free_fmc

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

void* malloc_fmc( size_t sz ); // only all FMC memory for now
void  free_fmc( void* ptr );

#define HX711_EXA_SCK_PIN  PG1
#define HX711_EXA_DAT_PIN  PG2

// 0 = DEVICE_FS, 1 = DEVICE_HS
#define BOARD_USB_DEFAULT_TYPE       1
#define BOARD_USB_DEFAULT_INSTANCE   USB_OTG_HS
#define BOARD_USB_DEFAULT_GPIO       GpioB
#define BOARD_USB_DEFAULT_DPDM_PINS  PinMask( GPIO_PIN_14 | GPIO_PIN_15 )
#define BOARD_USB_DEFAULT_VBUS_PIN   PB13
#define BOARD_USB_DEFAULT_ID_PIN     PB12
#define BOARD_USB_DEFAULT_GPIO_AF    GPIO_AF12_OTG_HS_FS
#define BOARD_USB_DEFAULT_ENABLE     __GPIOB_CLK_ENABLE();  __HAL_RCC_SYSCFG_CLK_ENABLE(); __HAL_RCC_USB_OTG_HS_CLK_ENABLE();
#define BOARD_USB_DEFAULT_DISABLE    __HAL_RCC_USB_OTG_HS_CLK_DISABLE();
#define BOARD_USB_DEFAULT_IRQ        OTG_HS_IRQn
#define BOARD_USB_DEFAULT_IRQHANDLER OTG_HS_IRQHandler
#define BOARD_USB_DEFAULT_IRQ_PRTY   14
// TODO: more params, FS/HS, see f429i_disc

#define BOARD_CONSOLE_DEFINES         UART_CONSOLE_DEFINES( USART1 );
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( USART1 );
#define BOARD_PROLOG                  STD_PROLOG_UART;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS;
#define BOARD_POST_INIT_BLINK         delay_ms( PROLOG_LED_TIME ); leds.write( 0_mask ); delay_ms( PROLOG_LED_TIME );

#endif
