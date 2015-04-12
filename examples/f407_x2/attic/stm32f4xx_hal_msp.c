/**
  * File Name          : stm32f4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  */

#include "stm32f4xx_hal.h"

void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

// void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
// {
//
//   GPIO_InitTypeDef GPIO_InitStruct;
//   if(hi2c->Instance==I2C1)
//   {
//     /* Peripheral clock enable */
//     __I2C1_CLK_ENABLE();
//
//     /**I2C1 GPIO Configuration
//     PB6     ------> I2C1_SCL
//     PB7     ------> I2C1_SDA
//     */
//     GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
//     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//     /* Peripheral interrupt init*/
//     /* Sets the priority grouping field */
//     HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
//     HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
//     HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
//   }
//
// }

// void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
// {
//
//   if(hi2c->Instance==I2C1)
//   {
//     /* Peripheral clock disable */
//     __I2C1_CLK_DISABLE();
//
//     /**I2C1 GPIO Configuration
//     PB6     ------> I2C1_SCL
//     PB7     ------> I2C1_SDA
//     */
//     HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);
//
//     /* Peripheral interrupt Deinit*/
//     HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
//   }
//
// }

// void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
// {
//
//   GPIO_InitTypeDef GPIO_InitStruct;
//   if(hspi->Instance==SPI1)
//   {
//     /* Peripheral clock enable */
//     __SPI1_CLK_ENABLE();
//
//     /**SPI1 GPIO Configuration
//     PA5     ------> SPI1_SCK
//     PA6     ------> SPI1_MISO
//     PA7     ------> SPI1_MOSI
//     */
//     GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
//     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//   }
//
// }

// void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
// {
//
//   if(hspi->Instance==SPI1)
//   {
//     /* Peripheral clock disable */
//     __SPI1_CLK_DISABLE();
//
//     /**SPI1 GPIO Configuration
//     PA5     ------> SPI1_SCK
//     PA6     ------> SPI1_MISO
//     PA7     ------> SPI1_MOSI
//     */
//     HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
//
//   }
//
// }

// void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
// {
//
//   GPIO_InitTypeDef GPIO_InitStruct;
//   if(htim_pwm->Instance==TIM1)
//   {
//     /* Peripheral clock enable */
//     __TIM1_CLK_ENABLE();
//
//     /**TIM1 GPIO Configuration
//     PE9     ------> TIM1_CH1
//     PE11     ------> TIM1_CH2
//     PE13     ------> TIM1_CH3
//     PE14     ------> TIM1_CH4
//     */
//     GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_11;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//     GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//     GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
//     HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
//
//     GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//     GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
//     HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
//
//     /* Peripheral interrupt init*/
//     /* Sets the priority grouping field */
//     HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
//     HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 0, 0);
//     HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
//   }
//
// }

// void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
// {
//
//   if(htim_pwm->Instance==TIM1)
//   {
//     /* Peripheral clock disable */
//     __TIM1_CLK_DISABLE();
//
//     /**TIM1 GPIO Configuration
//     PE9     ------> TIM1_CH1
//     PE11     ------> TIM1_CH2
//     PE13     ------> TIM1_CH3
//     PE14     ------> TIM1_CH4
//     */
//     HAL_GPIO_DeInit(GPIOE, GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_13|GPIO_PIN_14);
//
//     /* Peripheral interrupt Deinit*/
//     HAL_NVIC_DisableIRQ(TIM1_BRK_TIM9_IRQn);
//   }
//
// }

// void HAL_UART_MspInit(UART_HandleTypeDef* huart)
// {
//
//   GPIO_InitTypeDef GPIO_InitStruct;
//   if(huart->Instance==USART2)
//   {
//     /* Peripheral clock enable */
//     __USART2_CLK_ENABLE();
//
//     /**USART2 GPIO Configuration
//     PA2     ------> USART2_TX
//     PA3     ------> USART2_RX
//     */
//     GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
//     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//     /* Peripheral interrupt init*/
//     /* Sets the priority grouping field */
//     HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
//     HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
//     HAL_NVIC_EnableIRQ(USART2_IRQn);
//   }
//
// }

// void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
// {
//
//   if(huart->Instance==USART2)
//   {
//     /* Peripheral clock disable */
//     __USART2_CLK_DISABLE();
//
//     /**USART2 GPIO Configuration
//     PA2     ------> USART2_TX
//     PA3     ------> USART2_RX
//     */
//     HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
//
//     /* Peripheral interrupt Deinit*/
//     HAL_NVIC_DisableIRQ(USART2_IRQn);
//   }
//
// }

