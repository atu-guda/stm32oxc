/*
 * =====================================================================================
 *
 *       Filename:  oxc_archdef.h
 *
 *    Description:  arch-dependent definitions and includes 
 *
 *        Version:  1.0
 *        Created:  2019-07-31 00:11:52
 *       Revision:  none
 *
 *         Author:  Anton Guda (atu), atu@nmetau.edu.ua
 *
 * =====================================================================================
 */
#ifndef _OXC_ARCHDEF_H
#define _OXC_ARCHDEF_H

#if defined (STM32F0)
 #include <stm32f0xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BRR
 #define RESET_BIT_SHIFT 0
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_HIGH
 #define GPIO_EN_REG      AHBENR
 #define GPIO_EN_BIT0 RCC_AHBENR_GPIOAEN
 #define EXTICFG_PLACE SYSCFG
 #define EXTIREG_RTSR RTSR
 #define EXTIREG_FTSR FTSR
 #define EXTIREG_IMR  IMR
 #define EXTI_CFG_BITS 4

#elif defined (STM32F1)
 #include <stm32f1xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BRR
 #define RESET_BIT_SHIFT 0
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_HIGH
 #define GPIO_EN_REG      APB2ENR
 #define GPIO_EN_BIT0 RCC_APB2ENR_IOPAEN
 #define EXTICFG_PLACE AFIO
 #define EXTIREG_RTSR RTSR
 #define EXTIREG_FTSR FTSR
 #define EXTIREG_IMR  IMR
 #define EXTI_CFG_BITS 4

#elif defined (STM32F2)
 #include <stm32f2xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BSRR
 #define RESET_BIT_SHIFT 16
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_VERY_HIGH
 #define GPIO_EN_REG      AHB1ENR
 #define GPIO_EN_BIT0 RCC_AHB1ENR_GPIOAEN
 #define EXTICFG_PLACE SYSCFG
 #define EXTIREG_RTSR RTSR
 #define EXTIREG_FTSR FTSR
 #define EXTIREG_IMR  IMR
 #define EXTI_CFG_BITS 4

#elif defined (STM32F3)
 #include <stm32f3xx_hal.h>
 #include <Legacy/stm32_hal_legacy.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BRR
 #define RESET_BIT_SHIFT 0
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_HIGH
 #define GPIO_EN_REG      AHBENR
 #define GPIO_EN_BIT0 RCC_AHBENR_GPIOAEN
 #define EXTICFG_PLACE SYSCFG
 #define EXTIREG_RTSR RTSR
 #define EXTIREG_FTSR FTSR
 #define EXTIREG_IMR  IMR
 #define EXTI_CFG_BITS 4

#elif defined (STM32F4)
 #include <stm32f4xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BSRR
 #define RESET_BIT_SHIFT 16
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_VERY_HIGH
 #define GPIO_EN_REG      AHB1ENR
 #define GPIO_EN_BIT0 RCC_AHB1ENR_GPIOAEN
 #define EXTICFG_PLACE SYSCFG
 #define EXTIREG_RTSR RTSR
 #define EXTIREG_FTSR FTSR
 #define EXTIREG_IMR  IMR
 #define EXTI_CFG_BITS 4

#elif defined(STM32F7)
 #include <stm32f7xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BSRR
 #define RESET_BIT_SHIFT 16
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_VERY_HIGH
 #define GPIO_EN_REG      AHB1ENR
 #define GPIO_EN_BIT0 RCC_AHB1ENR_GPIOAEN
 #define EXTICFG_PLACE SYSCFG
 #define EXTIREG_RTSR RTSR
 #define EXTIREG_FTSR FTSR
 #define EXTIREG_IMR  IMR
 #define EXTI_CFG_BITS 4

#elif defined(STM32H5)
 #include <stm32h5xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BSRR
 #define RESET_BIT_SHIFT 16
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_VERY_HIGH
 #define GPIO_EN_REG      AHB2ENR
 #define GPIO_EN_BIT0 RCC_AHB2ENR_GPIOAEN
 #define EXTICFG_PLACE EXTI
 #define EXTIREG_RTSR RTSR1
 #define EXTIREG_FTSR FTSR1
 #define EXTIREG_IMR  IMR1
 #define EXTI_CFG_BITS 8

#elif defined(STM32H7)
 #include <stm32h7xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BSRR
 #define RESET_BIT_SHIFT 16
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_VERY_HIGH
 #define GPIO_EN_REG      AHB4ENR
 #define GPIO_EN_BIT0 RCC_AHB4ENR_GPIOAEN
 #define EXTICFG_PLACE SYSCFG
 #define EXTIREG_RTSR RTSR1
 #define EXTIREG_FTSR FTSR1
 #define EXTIREG_IMR  IMR1
 #define EXTI_CFG_BITS 4

#elif defined (STM32G4)
 #include <stm32g4xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BSRR
 #define RESET_BIT_SHIFT 16
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_VERY_HIGH
 #define GPIO_EN_REG      AHB2ENR
 #define GPIO_EN_BIT0 RCC_AHB2ENR_GPIOAEN
 #define EXTICFG_PLACE SYSCFG
 #define EXTIREG_RTSR RTSR1
 #define EXTIREG_FTSR FTSR1
 #define EXTIREG_IMR  IMR1
 #define EXTI_CFG_BITS 4
#else
  #error "Unsupported MCU"
#endif


#endif

