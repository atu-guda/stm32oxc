/**
  * File Name          : stm32f4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  */

// #include <stm32f4xx_hal.h>
// #include <usbd_core.h>

#include <oxc_base.h>
#include <oxc_gpio.h>

void default_USBFS_MspInit(void);

void HAL_PCD_MspInit( PCD_HandleTypeDef *hpcd UNUSED_ARG )
{
  default_USBFS_MspInit();
}

void HAL_I2C_MspInit( I2C_HandleTypeDef* hi2c )
{
  GPIO_InitTypeDef gpi;
  leds.toggle( BIT2 );
  if( hi2c->Instance == I2C1 ) {
    leds.toggle( BIT0 );
    __I2C1_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();

    // PB6 --> I2C1_SCL
    // PB7 --> I2C1_SDA
    gpi.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpi.Mode = GPIO_MODE_AF_OD;
    gpi.Pull = GPIO_NOPULL;
    gpi.Speed = GPIO_SPEED_HIGH;
    gpi.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init( GPIOB, &gpi );

    // tmp: not now
    // HAL_NVIC_SetPriority( I2C1_EV_IRQn, 15, 0 ); // TODO: from FreeRTOS
    // HAL_NVIC_EnableIRQ( I2C1_EV_IRQn );
  }

}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if( hi2c->Instance==I2C1 )  {
    __I2C1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_6|GPIO_PIN_7 );
    HAL_NVIC_DisableIRQ( I2C1_EV_IRQn );
  }

}

