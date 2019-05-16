#include <oxc_gpio.h>

int i2c_default_init( I2C_HandleTypeDef &i2c, int speed )
{
  i2c.Instance              = BOARD_I2C_DEFAULT;
  #if defined (STM32F3) || defined (STM32F7)
  if( speed == 1000000 ) {
    i2c.Init.Timing           = BOARD_I2C_DEFAULT_TIMING_1M;
  } else if ( speed == 400000 ) {
    i2c.Init.Timing           = BOARD_I2C_DEFAULT_TIMING_400;
  } else {
    i2c.Init.Timing           = BOARD_I2C_DEFAULT_TIMING_100;
  }
  i2c.Init.OwnAddress2Masks   = I2C_OA2_NOMASK;
  #else
  i2c.Init.ClockSpeed         = speed;
  i2c.Init.DutyCycle          = I2C_DUTYCYCLE_16_9;
  #endif
  i2c.State                   = HAL_I2C_STATE_RESET;
  i2c.Init.AddressingMode     = I2C_ADDRESSINGMODE_7BIT;
  i2c.Init.DualAddressMode    = I2C_DUALADDRESS_DISABLE;
  i2c.Init.GeneralCallMode    = I2C_GENERALCALL_DISABLE;
  i2c.Init.NoStretchMode      = I2C_NOSTRETCH_DISABLE;
  i2c.Init.OwnAddress1        = 0;
  i2c.Init.OwnAddress2        = 0;
  if( HAL_I2C_Init( &i2c ) != HAL_OK ) {
    return 0;
  }

  #if defined (STM32F3) || defined (STM32F7)
  HAL_I2CEx_AnalogFilter_Config( &i2c, I2C_ANALOGFILTER_ENABLED );
  // HAL_I2CEx_ConfigDigitalFilter( &i2c, 0 );
  #endif
  return 1;
}


void HAL_I2C_MspInit( I2C_HandleTypeDef *i2c )
{
  GPIO_InitTypeDef gio;
  if( i2c->Instance == BOARD_I2C_DEFAULT ) {
    BOARD_I2C_DEFAULT_ENABLE;

    gio.Mode      = GPIO_MODE_AF_OD;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_MAX;
    #ifdef BOARD_I2C_DEFAULT_GPIO_AF
    gio.Alternate = BOARD_I2C_DEFAULT_GPIO_AF;
    #endif

    gio.Pin       = BOARD_I2C_DEFAULT_GPIO_PIN_SCL;
    HAL_GPIO_Init(  BOARD_I2C_DEFAULT_GPIO_SCL, &gio );
    gio.Pin       = BOARD_I2C_DEFAULT_GPIO_PIN_SDA;
    HAL_GPIO_Init(  BOARD_I2C_DEFAULT_GPIO_SDA, &gio );
  }
}

void HAL_I2C_MspDeInit( I2C_HandleTypeDef *i2c )
{
  if( i2c->Instance == BOARD_I2C_DEFAULT ) {
    BOARD_I2C_DEFAULT_DISABLE;
    HAL_GPIO_DeInit( BOARD_I2C_DEFAULT_GPIO_SCL, BOARD_I2C_DEFAULT_GPIO_PIN_SCL );
    HAL_GPIO_DeInit( BOARD_I2C_DEFAULT_GPIO_SDA, BOARD_I2C_DEFAULT_GPIO_PIN_SDA );
    HAL_NVIC_DisableIRQ( BOARD_I2C_DEFAULT_IRQ );
  }
}


