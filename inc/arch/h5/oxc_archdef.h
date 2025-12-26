/*
 * =====================================================================================
 *
 *       Filename:  inc/arch/h5/oxc_archdef.h
 *
 *    Description:  STM32H5xx arch-dependent definitions and includes
 *         Author:  Anton Guda (atu), atu@nmetau.edu.ua
 * =====================================================================================
 */
#ifndef _OXC_ARCHDEF_H
#define _OXC_ARCHDEF_H

#include <stm32h5xx_hal.h>


#define   SET_BIT_REG            BSRR
#define   RESET_BIT_REG          BSRR
#define   RESET_BIT_SHIFT        16
#define   GPIO_SPEED_MAX         GPIO_SPEED_FREQ_VERY_HIGH
#define   GPIO_EN_REG            AHB2ENR
#define   GPIO_EN_BIT0           RCC_AHB2ENR_GPIOAEN
#define   EXTICFG_PLACE          EXTI
#define   EXTIREG_RTSR           RTSR1
#define   EXTIREG_FTSR           FTSR1
#define   EXTIREG_IMR            IMR1
#define   EXTI_CFG_BITS          8


// used in inc/oxc_post_board_cfg.h
#define EXTI_HANDLER_0  EXTI0_IRQHandler
#define EXTI_HANDLER_1  EXTI1_IRQHandler
#define EXTI_HANDLER_2  EXTI2_IRQHandler
#define EXTI_HANDLER_3  EXTI3_IRQHandler
#define EXTI_HANDLER_4  EXTI4_IRQHandler
#define EXTI_HANDLER_5  EXTI5_IRQHandler
#define EXTI_HANDLER_6  EXTI6_IRQHandler
#define EXTI_HANDLER_7  EXTI7_IRQHandler
#define EXTI_HANDLER_8  EXTI8_IRQHandler
#define EXTI_HANDLER_9  EXTI9_IRQHandler
#define EXTI_HANDLER_10 EXTI10_IRQHandler
#define EXTI_HANDLER_11 EXTI11_IRQHandler
#define EXTI_HANDLER_12 EXTI12_IRQHandler
#define EXTI_HANDLER_13 EXTI13_IRQHandler
#define EXTI_HANDLER_14 EXTI14_IRQHandler
#define EXTI_HANDLER_15 EXTI15_IRQHandler
//
#define EXTI_IRQ_0  EXTI0_IRQn
#define EXTI_IRQ_1  EXTI1_IRQn
#define EXTI_IRQ_2  EXTI2_IRQn
#define EXTI_IRQ_3  EXTI3_IRQn
#define EXTI_IRQ_4  EXTI4_IRQn
#define EXTI_IRQ_5  EXTI5_IRQn
#define EXTI_IRQ_6  EXTI6_IRQn
#define EXTI_IRQ_7  EXTI7_IRQn
#define EXTI_IRQ_8  EXTI8_IRQn
#define EXTI_IRQ_9  EXTI9_IRQn
#define EXTI_IRQ_10 EXTI10_IRQn
#define EXTI_IRQ_11 EXTI11_IRQn
#define EXTI_IRQ_12 EXTI12_IRQn
#define EXTI_IRQ_13 EXTI13_IRQn
#define EXTI_IRQ_14 EXTI14_IRQn
#define EXTI_IRQ_15 EXTI15_IRQn


#endif

