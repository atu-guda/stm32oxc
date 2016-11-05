/**
  * File Name          : stm32f3xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  */

#include <stm32f3xx_hal.h>

#include <FreeRTOS.h>

void MX_I2C1_Init(I2C_HandleTypeDef &i2c )
{
  i2c.Instance = I2C1;
  i2c.Init.Timing = 0x10808DD3;
  i2c.Init.OwnAddress1 = 0;
  i2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  i2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  i2c.Init.OwnAddress2 = 0;
  i2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  i2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  i2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
  HAL_I2C_Init( &i2c );

  HAL_I2CEx_AnalogFilter_Config( &i2c, I2C_ANALOGFILTER_ENABLED );
}


void HAL_I2C_MspInit( I2C_HandleTypeDef *hi2c )
{
  GPIO_InitTypeDef gio;
  if( hi2c->Instance == I2C1 ) {
    __I2C1_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    // I2C1 GPIO B6 --> I2C1_SCL, B7 --> I2C1_SDA
    gio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gio.Mode = GPIO_MODE_AF_OD;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_HIGH;
    gio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gio);
  }
}

void HAL_I2C_MspDeInit( I2C_HandleTypeDef* hi2c )
{
  if( hi2c->Instance == I2C1 )
  {
    __I2C1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_6 | GPIO_PIN_7 );
  }
}



