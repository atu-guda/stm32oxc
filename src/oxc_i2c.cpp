#include <oxc_i2c.h>

void DevI2C::initI2C( uint32_t speed, uint8_t own_addr )
{
  resetDev();
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
  addr = effAddr( addr );
  HAL_StatusTypeDef rc = HAL_I2C_IsDeviceReady( i2ch, addr, 3, maxWait );
  return ( rc == HAL_OK );
}

int DevI2C::send( uint8_t ds, uint8_t addr )
{
  addr = effAddr( addr );
  HAL_StatusTypeDef rc = HAL_I2C_Master_Transmit( i2ch, addr, &ds, 1, maxWait );
  return ( rc == HAL_OK ) ? 1 : 0;
}

int  DevI2C::send( const uint8_t *ds, int ns, uint8_t addr )
{
  addr = effAddr( addr );
  if( ds == nullptr || ns < 1 ) {
    return 0;
  }
  HAL_StatusTypeDef rc = HAL_I2C_Master_Transmit( i2ch, addr, (uint8_t*)ds, ns, maxWait );
  return ( rc == HAL_OK ) ? ns : 0;
}

int  DevI2C::send_reg12( uint8_t reg,  const uint8_t *ds, int ns, bool is2byte, uint8_t addr )
{
  addr = effAddr( addr );
  if( ds == nullptr || ns < 1 ) {
    return 0;
  }
  auto szcmd = is2byte ? I2C_MEMADD_SIZE_16BIT: I2C_MEMADD_SIZE_8BIT;
  HAL_StatusTypeDef rc = HAL_I2C_Mem_Write( i2ch, addr, reg, szcmd,
            (uint8_t*)ds, ns, maxWait );
  return ( rc == HAL_OK ) ? ns : 0;
}



int  DevI2C::send_reg1( uint8_t reg,  const uint8_t *ds, int ns, uint8_t addr )
{
  return send_reg12( reg, ds, ns, false, addr );
}

int  DevI2C::send_reg2(  uint16_t reg, const uint8_t *ds, int ns, uint8_t addr )
{
  return send_reg12( reg, ds, ns, true, addr );
}

int  DevI2C::recv( uint8_t addr )
{
  addr = effAddr( addr );
  uint8_t v;
  HAL_StatusTypeDef rc = HAL_I2C_Master_Receive( i2ch, addr, (uint8_t*)(&v), 1, maxWait );
  return ( rc == HAL_OK ) ? v : -1;

}

int  DevI2C::recv( uint8_t *dd, int nd, uint8_t addr )
{
  addr = effAddr( addr );
  if( dd == nullptr || nd < 1 ) {
    return 0;
  }
  HAL_StatusTypeDef rc = HAL_I2C_Master_Receive( i2ch, addr, (uint8_t*)(dd), nd, maxWait );
  return ( rc == HAL_OK ) ? nd :0;
}


int  DevI2C::recv_reg12(  int8_t reg,  uint8_t *dd, int nd, bool is2byte, uint8_t addr )
{
  addr = effAddr( addr );
  if( dd == nullptr || nd < 1 ) {
    return 0;
  }
  auto szcmd = is2byte ? I2C_MEMADD_SIZE_16BIT: I2C_MEMADD_SIZE_8BIT;
  HAL_StatusTypeDef rc = HAL_I2C_Mem_Read( i2ch, addr, reg, szcmd,
      dd, nd, maxWait );
  return ( rc == HAL_OK ) ? nd :0;
}

int  DevI2C::recv_reg1(  int8_t reg,  uint8_t *dd, int nd, uint8_t addr )
{
  return recv_reg12( reg, dd, nd, false, addr );
}

int  DevI2C::recv_reg2(  int16_t reg, uint8_t *dd, int nd, uint8_t addr )
{
  return recv_reg12( reg, dd, nd, true, addr );
}

// --------------- I2CClient functions -------------------------


uint8_t I2CClient::recv_reg1_8bit(  int8_t reg,  uint8_t defVal )
{
  uint8_t v = defVal, vx;
  if( dev.recv_reg1( reg, &vx, 1, addr ) == 1 ) {
    v = vx;
  }
  return v;
}

uint16_t I2CClient::recv_reg1_16bit(  int8_t reg,  uint16_t defVal )
{
  uint16_t v = defVal, vx;
  if( dev.recv_reg1( reg, (uint8_t*)(&vx), 2, addr ) == 2 ) {
    v = vx;
  }
  return v;
}

uint16_t I2CClient::recv_reg1_16bit_rev(  int8_t reg,  uint16_t defVal )
{
  uint16_t v = defVal, vx;
  if( dev.recv_reg1( reg, (uint8_t*)(&vx), 2, addr ) == 2 ) {
    v = rev16( vx );
  }
  return v;
}


int I2CClient::recv_reg1_16bit_n(  int8_t reg, uint16_t *d, int n )
{
  return dev.recv_reg1( reg, (uint8_t*)(d), n*2, addr ) / 2;
}

int I2CClient::recv_reg1_16bit_n_rev(  int8_t reg, uint16_t *d, int n )
{
  int n_o = recv_reg1_16bit_n( reg, d, n );
  for( int i=0; i<n_o; ++i ) {
    d[i] = rev16( d[i] );
  }
  return n_o;
}


