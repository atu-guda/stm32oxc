#ifndef _BOARD_STM32F407V_BB0_H
#define _BOARD_STM32F407V_BB0_H

#define _BOARD_CFG_DEFINED

// definition of resoures for STM32F407VET black board
// headers must be included manualy in C/CPP file

#define DELAY_APPROX_COEFF  5010

#define def_stksz 512


// special LEDS B12:B15
#define BOARD_N_LEDS    4
#define BOARD_LEDS_OFS  12
#define BOARD_LEDS_GPIOX B

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

#define BOARD_BTN0_EXIST   1
#define BOARD_BTN0_GPIOX   C
#define BOARD_BTN0_N       7
#define BOARD_BTN0_ACTIVE_DOWN 0
#define BOARD_BTN0_IRQNAME EXTI9_5

#define BOARD_BTN1_EXIST   1
#define BOARD_BTN1_GPIOX   C
#define BOARD_BTN1_N       1
#define BOARD_BTN1_ACTIVE_DOWN 0
#define BOARD_BTN1_IRQNAME EXTI1


#define TIM_EXA        TIM1
#define TIM_EXA_STR    "TIM1"
#define TIM_EXA_GPIO   GpioE
#define TIM_EXA_PIN1   GPIO_PIN_9
#define TIM_EXA_PIN2   GPIO_PIN_11
#define TIM_EXA_PIN3   GPIO_PIN_13
#define TIM_EXA_PIN4   GPIO_PIN_14
#define TIM_EXA_PINS   ( TIM_EXA_PIN1 | TIM_EXA_PIN2 | TIM_EXA_PIN3 | TIM_EXA_PIN4 )
#define TIM_EXA_CLKEN  __GPIOE_CLK_ENABLE(); __TIM1_CLK_ENABLE();
#define TIM_EXA_CLKDIS __TIM1_CLK_DISABLE();
#define TIM_EXA_GPIOAF GPIO_AF1_TIM1
#define TIM_EXA_IRQ    TIM1_CC_IRQn
#define TIM_EXA_IRQHANDLER    TIM1_CC_IRQHandler

#define SD_EXA_CK_GPIO   GpioC
#define SD_EXA_CK_PIN    12
#define SD_EXA_D0_GPIO   GpioC
#define SD_EXA_D0_PIN    8
#define SD_EXA_CMD_GPIO  GpioD
#define SD_EXA_CMD_PIN   2
#define SD_EXA_CLKEN     __HAL_RCC_SDIO_CLK_ENABLE();  __HAL_RCC_GPIOC_CLK_ENABLE();  __HAL_RCC_GPIOD_CLK_ENABLE();
#define SD_EXA_CLKDIS    __HAL_RCC_SDIO_CLK_DISABLE();
#define SD_EXA_GPIOAF    GPIO_AF12_SDIO

#define BOARD_UART_DEFAULT            USART1
#define BOARD_UART_DEFAULT_GPIO       GpioA
#define BOARD_UART_DEFAULT_GPIO_PINS  ( GPIO_PIN_9 | GPIO_PIN_10 )
#define BOARD_UART_DEFAULT_GPIO_AF    GPIO_AF7_USART1
#define BOARD_UART_DEFAULT_ENABLE     __USART1_CLK_ENABLE(); __GPIOA_CLK_ENABLE();
#define BOARD_UART_DEFAULT_DISABLE    __USART1_CLK_DISABLE();
#define BOARD_UART_DEFAULT_IRQ        USART1_IRQn
#define BOARD_UART_DEFAULT_IRQHANDLER USART1_IRQHandler

#define BOARD_I2C_DEFAULT               I2C1
#define BOARD_I2C_DEFAULT_NAME          "I2C1"
#define BOARD_I2C_DEFAULT_SPEED         100000
#define BOARD_I2C_DEFAULT_GPIO_SCL      GpioB
#define BOARD_I2C_DEFAULT_GPIO_SDA      GpioB
#define BOARD_I2C_DEFAULT_GPIO_PIN_SCL  6
#define BOARD_I2C_DEFAULT_GPIO_PIN_SDA  7
#define BOARD_I2C_DEFAULT_GPIO_AF       GPIO_AF4_I2C1
#define BOARD_I2C_DEFAULT_ENABLE        __I2C1_CLK_ENABLE(); __GPIOB_CLK_ENABLE();
#define BOARD_I2C_DEFAULT_DISABLE       __I2C1_CLK_DISABLE();
#define BOARD_I2C_DEFAULT_IRQ           I2C1_EV_IRQn
#define BOARD_I2C_DEFAULT_IRQHANDLER    I2C1_EV_IRQHandler

#define BOARD_IN0_GPIO                  GpioB
#define BOARD_IN0_PINNUM                8
#define BOARD_IN1_GPIO                  GpioB
#define BOARD_IN1_PINNUM                9
#define BOARD_IN2_GPIO                  GpioB
#define BOARD_IN2_PINNUM                10


#define BOARD_SPI_DEFAULT               SPI2
#define BOARD_SPI_DEFAULT_NAME          "SPI2"
#define BOARD_SPI_DEFAULT_GPIO_ALL      GpioB
//#define BOARD_SPI_DEFAULT_GPIO_SCK      GpioB
#define BOARD_SPI_DEFAULT_GPIO_PIN_SCK  GPIO_PIN_13
//#define BOARD_SPI_DEFAULT_GPIO_MISO     GpioB
#define BOARD_SPI_DEFAULT_GPIO_PIN_MISO GPIO_PIN_14
//#define BOARD_SPI_DEFAULT_GPIO_MOSI     GpioB
#define BOARD_SPI_DEFAULT_GPIO_PIN_MOSI GPIO_PIN_15
#define BOARD_SPI_DEFAULT_GPIO_SNSS     GpioB
// here number, as input to PinsOut
#define BOARD_SPI_DEFAULT_GPIO_PIN_SNSS 12
#define BOARD_SPI_DEFAULT_GPIO_EXT1     GpioE
#define BOARD_SPI_DEFAULT_GPIO_PIN_EXT1 4
#define BOARD_SPI_DEFAULT_GPIO_EXT2     GpioE
#define BOARD_SPI_DEFAULT_GPIO_PIN_EXT2 5
#define BOARD_SPI_DEFAULT_GPIO_AF       GPIO_AF5_SPI1
#define BOARD_SPI_DEFAULT_ENABLE        __SPI2_CLK_ENABLE(); __GPIOB_CLK_ENABLE(); __GPIOC_CLK_ENABLE(); __GPIOE_CLK_ENABLE();
#define BOARD_SPI_DEFAULT_DISABLE       __SPI2_CLK_DISABLE();
#define BOARD_SPI_DEFAULT_IRQ           SPI2_IRQn
#define BOARD_SPI_DEFAULT_IRQHANDLER    SPI2_IRQHandler
#define BOARD_SPI_BAUDRATEPRESCALER_FAST SPI_BAUDRATEPRESCALER_2


#define BOARD_MOTOR_DEFAULT_GPIO        GpioE
#define BOARD_MOTOR_DEFAULT_PIN0        0

#define BOARD_1W_DEFAULT_GPIO           GpioE
#define BOARD_1W_DEFAULT_PIN            GPIO_PIN_15

// ADC: A0-A3 (0-3)
#define BOARD_ADC_DEFAULT_DEV           ADC1
#define BOARD_ADC_DEFAULT_EN            __HAL_RCC_ADC1_CLK_ENABLE();
#define BOARD_ADC_DEFAULT_DIS           __HAL_RCC_ADC1_CLK_DISABLE();
#define BOARD_ADC_DEFAULT_GPIO0         GpioA
#define BOARD_ADC_DEFAULT_PIN0          0
#define BOARD_ADC_DEFAULT_CH0           ADC_CHANNEL_0
#define BOARD_ADC_DEFAULT_GPIO1         GpioA
#define BOARD_ADC_DEFAULT_PIN1          1
#define BOARD_ADC_DEFAULT_CH1           ADC_CHANNEL_1
#define BOARD_ADC_DEFAULT_GPIO2         GpioA
#define BOARD_ADC_DEFAULT_PIN2          2
#define BOARD_ADC_DEFAULT_CH2           ADC_CHANNEL_2
#define BOARD_ADC_DEFAULT_GPIO3         GpioA
#define BOARD_ADC_DEFAULT_PIN3          3
#define BOARD_ADC_DEFAULT_CH3           ADC_CHANNEL_3
#define BOARD_ADC_MEM_MAX               (1024*64)
// #define BOARD_ADC_MEM_MAX_FMC           (1024*1024*8)
#define BOARD_ADC_COEFF                 3302000
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

// 0 = DEVICE_FS, 1 = DEVICE_HS
#define BOARD_USB_DEFAULT_TYPE       0
#define BOARD_USB_DEFAULT_INSTANCE   USB_OTG_FS
#define BOARD_USB_DEFAULT_GPIO       GpioA
#define BOARD_USB_DEFAULT_DPDM_PINS  ( GPIO_PIN_11 | GPIO_PIN_12 )
// #define BOARD_USB_DEFAULT_VBUS_PIN   GPIO_PIN_9
// #define BOARD_USB_DEFAULT_ID_PIN     GPIO_PIN_10
#define BOARD_USB_DEFAULT_GPIO_AF    GPIO_AF10_OTG_FS
#define BOARD_USB_DEFAULT_ENABLE     __GPIOA_CLK_ENABLE(); __HAL_RCC_USB_OTG_FS_CLK_ENABLE(); __HAL_RCC_SYSCFG_CLK_ENABLE();
#define BOARD_USB_DEFAULT_DISABLE    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
#define BOARD_USB_DEFAULT_IRQ        OTG_FS_IRQn
#define BOARD_USB_DEFAULT_IRQHANDLER OTG_FS_IRQHandler
#define BOARD_USB_DEFAULT_IRQ_PRTY   14

#define BOARD_CONSOLE_DEFINES         USBCDC_CONSOLE_DEFINES;
#define BOARD_CONSOLE_DEFINES_UART    UART_CONSOLE_DEFINES( USART1 );
#define BOARD_PROLOG                  STD_PROLOG_USBCDC;
#define BOARD_CREATE_STD_TASKS        CREATE_STD_TASKS;
#define BOARD_POST_INIT_BLINK         delay_ms( PROLOG_LED_TIME ); leds.write( 0x00 ); delay_ms( PROLOG_LED_TIME );

#endif
