#include <oxc_i2c.h>

void DevI2C::initI2C( uint32_t speed, uint8_t own_addr )
{
  // i2c.Instance = I2C1;
  // i2c.Init.Timing = 0x10808DD3;
  // i2c.Init.OwnAddress1 = 0;
  // i2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  // i2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  // i2c.Init.OwnAddress2 = 0;
  // i2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  // i2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  // i2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
  // HAL_I2C_Init( i2ch );
  __HAL_I2C_ENABLE( i2ch );
}

void DevI2C::deInit()
{
  __HAL_I2C_DISABLE( i2ch );
  HAL_I2C_DeInit( i2ch );
}

void DevI2C::resetDev()
{
  __HAL_I2C_DISABLE( i2ch );
  delay_ms( 10 );
  __HAL_I2C_ENABLE( i2ch );
  delay_ms( 10 );
}

bool DevI2C::ping( uint8_t addr )
{
  if( addr == 0 ) {
    addr = addr2;
  } else {
    addr = addr << 1;
  }
  HAL_StatusTypeDef rc = HAL_I2C_IsDeviceReady( i2ch, addr, 3, maxWait );
  return ( rc == HAL_OK );
}

int DevI2C::send( uint8_t ds )
{
  HAL_StatusTypeDef rc = HAL_I2C_Master_Transmit( i2ch, addr2, &ds, 1, maxWait );
  return ( rc == HAL_OK ) ? 1 : 0;
}

int  DevI2C::send( const uint8_t *ds, int ns )
{
  if( *ds || ns < 1 ) {
    return 0;
  }
  HAL_StatusTypeDef rc = HAL_I2C_Master_Transmit( i2ch, addr2, (uint8_t*)ds, ns, maxWait );
  return ( rc == HAL_OK ) ? ns : 0;
}


int  DevI2C::send_reg1( uint8_t reg,  const uint8_t *ds, int ns )
{
  HAL_StatusTypeDef rc = HAL_I2C_Mem_Write( i2ch, addr2, reg, I2C_MEMADD_SIZE_8BIT,
            (uint8_t*)ds, ns, maxWait );
  return ( rc == HAL_OK ) ? ns : 0;
}

int  DevI2C::send_reg2(  uint16_t reg, const uint8_t *ds, int ns )
{
  HAL_StatusTypeDef rc = HAL_I2C_Mem_Write( i2ch, addr2, reg, I2C_MEMADD_SIZE_16BIT,
            (uint8_t*)ds, ns, maxWait );
  return ( rc == HAL_OK ) ? ns : 0;
}

int  DevI2C::recv()
{
  uint8_t v;
  HAL_StatusTypeDef rc = HAL_I2C_Master_Receive( i2ch, addr2, (uint8_t*)(&v), 1, maxWait );
  return ( rc == HAL_OK ) ? v : -1;

}

int  DevI2C::recv( uint8_t *dd, int nd )
{
  HAL_StatusTypeDef rc = HAL_I2C_Master_Receive( i2ch, addr2, (uint8_t*)(dd), nd, maxWait );
  return ( rc == HAL_OK ) ? nd :0;
}


int  DevI2C::recv_reg1(  int8_t reg,  uint8_t *dd, int nd )
{
  HAL_StatusTypeDef rc = HAL_I2C_Mem_Read( i2ch, addr2, reg, I2C_MEMADD_SIZE_8BIT,
      dd, nd, maxWait );
  return ( rc == HAL_OK ) ? nd :0;
}

int  DevI2C::recv_reg2(  int16_t reg, uint8_t *dd, int nd )
{
  HAL_StatusTypeDef rc = HAL_I2C_Mem_Read( i2ch, addr2, reg, I2C_MEMADD_SIZE_16BIT,
      dd, nd, maxWait );
  return ( rc == HAL_OK ) ? nd :0;
}

