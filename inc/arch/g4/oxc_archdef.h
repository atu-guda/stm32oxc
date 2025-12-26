/*
 * =====================================================================================
 *
 *       Filename:  inc/arch/r4/oxc_archdef.h
 *
 *    Description:  STM32G4xx arch-dependent definitions and includes
 *         Author:  Anton Guda (atu), atu@nmetau.edu.ua
 * =====================================================================================
 */
#ifndef _OXC_ARCHDEF_H
#define _OXC_ARCHDEF_H

#include <stm32g4xx_hal.h>


#define   SET_BIT_REG            BSRR
#define   RESET_BIT_REG          BSRR
#define   RESET_BIT_SHIFT        16
#define   GPIO_SPEED_MAX         GPIO_SPEED_FREQ_VERY_HIGH
#define   GPIO_EN_REG            AHB2ENR
#define   GPIO_EN_BIT0           RCC_AHB2ENR_GPIOAEN
#define   EXTICFG_PLACE          SYSCFG
#define   EXTIREG_RTSR           RTSR1
#define   EXTIREG_FTSR           FTSR1
#define   EXTIREG_IMR            IMR1
#define   EXTI_CFG_BITS          4


// used in inc/oxc_post_board_cfg.h
#define EXTI_HANDLER_0  EXTI0_IRQHandler
#define EXTI_HANDLER_1  EXTI1_IRQHandler
#define EXTI_HANDLER_2  EXTI2_IRQHandler
#define EXTI_HANDLER_3  EXTI3_IRQHandler
#define EXTI_HANDLER_4  EXTI4_IRQHandler
#define EXTI_HANDLER_5  EXTI9_5_IRQHandler
#define EXTI_HANDLER_6  EXTI9_5_IRQHandler
#define EXTI_HANDLER_7  EXTI9_5_IRQHandler
#define EXTI_HANDLER_8  EXTI9_5_IRQHandler
#define EXTI_HANDLER_9  EXTI9_5_IRQHandler
#define EXTI_HANDLER_10 EXTI15_10_IRQHandler
#define EXTI_HANDLER_11 EXTI15_10_IRQHandler
#define EXTI_HANDLER_12 EXTI15_10_IRQHandler
#define EXTI_HANDLER_13 EXTI15_10_IRQHandler
#define EXTI_HANDLER_14 EXTI15_10_IRQHandler
#define EXTI_HANDLER_15 EXTI15_10_IRQHandler
//
#define EXTI_IRQ_0  EXTI0_IRQn
#define EXTI_IRQ_1  EXTI1_IRQn
#define EXTI_IRQ_2  EXTI2_IRQn
#define EXTI_IRQ_3  EXTI3_IRQn
#define EXTI_IRQ_4  EXTI4_IRQn
#define EXTI_IRQ_5  EXTI9_5_IRQn
#define EXTI_IRQ_6  EXTI9_5_IRQn
#define EXTI_IRQ_7  EXTI9_5_IRQn
#define EXTI_IRQ_8  EXTI9_5_IRQn
#define EXTI_IRQ_9  EXTI9_5_IRQn
#define EXTI_IRQ_10 EXTI15_10_IRQn
#define EXTI_IRQ_11 EXTI15_10_IRQn
#define EXTI_IRQ_12 EXTI15_10_IRQn
#define EXTI_IRQ_13 EXTI15_10_IRQn
#define EXTI_IRQ_14 EXTI15_10_IRQn
#define EXTI_IRQ_15 EXTI15_10_IRQn


#endif

