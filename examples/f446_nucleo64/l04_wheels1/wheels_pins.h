#ifndef _WHEELS_PINS_H
#define _WHEELS_PINS_H

// on-board button
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

// my LEDs - auto init by PinsOut
#define LED0_Pin GPIO_PIN_0
#define LED0_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_2
#define LED2_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_3
#define LED3_GPIO_Port GPIOC

#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA

// T1: 1,2 - motor PWM, 3 - US pulse, 4 - US echo
#define T1_1_M_Right_Pin GPIO_PIN_8
#define T1_1_M_Right_GPIO_Port GPIOA
#define T1_2_M_Left_Pin GPIO_PIN_9
#define T1_2_M_Left_GPIO_Port GPIOA
#define T1_3_US_Pulse_Pin GPIO_PIN_10
#define T1_3_US_Pulse_GPIO_Port GPIOA
#define T1_4_US_Echo_Pin GPIO_PIN_11
#define T1_4_US_Echo_GPIO_Port GPIOA
#define T1_ALL_GPIO_Port GPIOA

// TIM3: 1 = A6 = right wheel counter
#define T3_1_M_count_R_Pin GPIO_PIN_6
#define T3_1_M_count_R_GPIO_Port GPIOA

// TIM4: 1 = B6 = left wheel counter
#define T4_1_M_count_l_Pin GPIO_PIN_6
#define T4_1_M_count_l_GPIO_Port GPIOB

// TIM14: 1 = A7 = servo contorl PIN
#define T14_1_servo_Pin GPIO_PIN_7
#define T14_1_servo_GPIO_Port GPIOA

// Motor direction
#define M_dir_r_Pin GPIO_PIN_5
#define M_dir_r_GPIO_Port GPIOC
#define M_dir_rn_Pin GPIO_PIN_6
#define M_dir_rn_GPIO_Port GPIOC
#define M_dir_aux_Pin GPIO_PIN_7
#define M_dir_aux_GPIO_Port GPIOC
#define M_dir_l_Pin GPIO_PIN_8
#define M_dir_l_GPIO_Port GPIOC
#define M_dir_ln_Pin GPIO_PIN_9
#define M_dir_ln_GPIO_Port GPIOC

// IR proxy sensors pins
#define prox_fr_Pin GPIO_PIN_12
#define prox_fr_GPIO_Port GPIOB
#define prox_frB13_Pin GPIO_PIN_13
#define prox_frB13_GPIO_Port GPIOB
#define prox_br_Pin GPIO_PIN_14
#define prox_br_GPIO_Port GPIOB
#define prox_bl_Pin GPIO_PIN_15
#define prox_bl_GPIO_Port GPIOB

// system
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB


#endif

