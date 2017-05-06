#include <oxc_base.h>
#include <oxc_gpio.h>

void MX_I2C1_Init( I2C_HandleTypeDef &i2c )
{
  i2c.Instance              = I2C1;
  i2c.Init.Timing           = 0x30C0EDFF; // 100 kHz over 200 MHz
  i2c.State                 = HAL_I2C_STATE_RESET;
  i2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
  i2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
  // i2c.Init.DutyCycle        = I2C_DUTYCYCLE_16_9;
  i2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
  i2c.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
  i2c.Init.OwnAddress1      = 0;
  i2c.Init.OwnAddress2      = 0;
  i2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  HAL_I2C_Init( &i2c );

  HAL_I2CEx_AnalogFilter_Config( &i2c, I2C_ANALOGFILTER_ENABLED );
  HAL_I2CEx_ConfigDigitalFilter( &i2c, 0 );
}


void HAL_I2C_MspInit( I2C_HandleTypeDef *i2c )
{
  GPIO_InitTypeDef gio;
  if( i2c->Instance == I2C1 ) {
    __I2C1_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();

    // B8 --> I2C1_SCL
    // B9 --> I2C1_SDA
    gio.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
    gio.Mode      = GPIO_MODE_AF_OD;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_MAX;
    gio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init( GPIOB, &gio );
  }
}

void HAL_I2C_MspDeInit( I2C_HandleTypeDef *i2c )
{
  if( i2c->Instance == I2C1 ) {
    __I2C1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_8 | GPIO_PIN_8 );
    HAL_NVIC_DisableIRQ( I2C1_EV_IRQn );
  }
}


