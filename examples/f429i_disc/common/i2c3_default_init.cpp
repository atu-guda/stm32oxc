/**
  * File Name          : i2c3_default_init.cpp
  * Description        : Default initialization for I2C3 for examples
  */

#include <oxc_base.h>
#include <oxc_gpio.h>

void MX_I2C3_Init( I2C_HandleTypeDef &i2c, uint32_t speed  )
{
  i2c.Instance             = I2C3;
  i2c.State                = HAL_I2C_STATE_RESET;
  i2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  i2c.Init.ClockSpeed      = speed;
  i2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  i2c.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
  i2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  i2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  i2c.Init.OwnAddress1     = 0;
  i2c.Init.OwnAddress2     = 0;
  HAL_I2C_Init( &i2c );
}

void HAL_I2C_MspInit( I2C_HandleTypeDef* hi2c )
{
  GPIO_InitTypeDef gpi;
  // leds.toggle( BIT2 );
  if( hi2c->Instance == I2C3 ) {
    // leds.toggle( BIT0 );
    __I2C3_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();

    // A8 --> I2C3_SCL
    // C9 --> I2C3_SDA
    gpi.Pin = GPIO_PIN_8;
    gpi.Mode = GPIO_MODE_AF_OD;
    gpi.Pull = GPIO_NOPULL;
    gpi.Speed = GPIO_SPEED_HIGH;
    gpi.Alternate = GPIO_AF4_I2C3;
    HAL_GPIO_Init( GPIOA, &gpi );
    gpi.Pin = GPIO_PIN_9;
    HAL_GPIO_Init( GPIOC, &gpi );

    // tmp: not now
    // HAL_NVIC_SetPriority( I2C3_EV_IRQn, 15, 0 ); // TODO: from FreeRTOS
    // HAL_NVIC_EnableIRQ( I2C3_EV_IRQn );
  }

}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if( hi2c->Instance==I2C3 )  {
    __I2C3_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_8 );
    HAL_GPIO_DeInit( GPIOC, GPIO_PIN_9 );
    HAL_NVIC_DisableIRQ( I2C3_EV_IRQn );
  }

}
