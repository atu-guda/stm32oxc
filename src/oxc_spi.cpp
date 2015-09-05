#include <oxc_spi.h>

void DevSPI::initSPI()
{
  __HAL_SPI_ENABLE( spi );
  if( nss_pin ) {
    nss_pin->initHW();
  }
  nss_post_cond();
}

void DevSPI::deInit()
{
  __HAL_SPI_DISABLE( spi );
  HAL_SPI_DeInit( spi );
}

void DevSPI::resetDev()
{
  __HAL_SPI_DISABLE( spi );
  delay_ms( 1 );
  __HAL_SPI_ENABLE( spi );
  nss_post_cond();
  delay_ms( 1 );
}

void DevSPI::nss_pre()
{
  if( !nss_pin ) { return; };
  if( inv_nss ) {
    nss_pin->set( 1 );
  } else {
    nss_pin->reset( 1 );
  }
  delay_bad_mcs( tss_delay_mcs );
}

void DevSPI::nss_post()
{
  if( !nss_pin ) { return; };
  if( inv_nss ) {
    nss_pin->reset( 1 );
  } else {
    nss_pin->set( 1 );
  }
  delay_bad_mcs( tss_delay_mcs );
}

HAL_StatusTypeDef DevSPI::waitForFlag(  uint32_t flag, uint32_t val, int ticks )
{
  if( ticks < 0 ) { ticks = maxWait; };

  uint32_t st = HAL_GetTick(), ct = 0;
  for( ; ct < (unsigned)ticks; ct = HAL_GetTick() - st ) {
    if( ( spi->Instance->SR & flag ) == val )  {
      return HAL_OK;
    }
    taskYieldFun();
  }

  // is really need?
  __HAL_SPI_DISABLE_IT( spi, (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR ) );
  if( spi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLED )
  {
    __HAL_SPI_RESET_CRC( spi );
  }
  spi->State= HAL_SPI_STATE_READY;

  __HAL_UNLOCK( spi );

  return HAL_TIMEOUT;
}


int DevSPI::send( uint8_t ds )
{
  return send( &ds, 1 );
}

int  DevSPI::send( const uint8_t *ds, int ns )
{
  if( ds == nullptr || ns < 1 ) {
    return 0;
  }

  nss_pre_cond();
  HAL_StatusTypeDef rc = HAL_SPI_Transmit( spi, (uint8_t*)ds, ns, maxWait );
  nss_post_cond();

  return ( rc == HAL_OK ) ? ns : 0;
}


int  DevSPI::recv()
{
  uint8_t v;
  int n = recv( &v, 1 );
  return ( n == 1 ) ? v : 0;
}

int  DevSPI::recv( uint8_t *dd, int nd )
{
  if( dd == nullptr || nd < 1 ) {
    return 0;
  }

  nss_pre_cond();
  HAL_StatusTypeDef rc = HAL_SPI_Receive( spi, (uint8_t*)(dd), nd, maxWait );
  nss_post_cond();

  return ( rc == HAL_OK ) ? nd : 0;
}

int  DevSPI::send_recv( const uint8_t *ds, int ns, uint8_t *dd, int nd )
{
  if( ds == nullptr || dd == nullptr ) {
    return 0;
  }

  nss_pre_cond();
  HAL_StatusTypeDef rc = HAL_SPI_Transmit( spi, (uint8_t*)ds, ns, maxWait );
  if( rc == HAL_OK ) {
    rc = HAL_SPI_Receive( spi, (uint8_t*)(dd), nd, maxWait );
  }
  nss_post_cond();

  return ( rc == HAL_OK ) ? nd : 0;
}

int  DevSPI::send2( const uint8_t *ds1, int ns1, const uint8_t *ds2, int ns2 )
{
  if( ds1 == nullptr || ns1 < 1 || ds2 == nullptr || ns2 < 1 ) {
    return 0;
  }

  nss_pre_cond();
  HAL_StatusTypeDef rc = HAL_SPI_Transmit( spi, (uint8_t*)ds1, ns1, maxWait );
  if( rc == HAL_OK ) {
    rc = HAL_SPI_Transmit( spi, (uint8_t*)(ds2), ns2, maxWait );
  }
  nss_post_cond();

  return ( rc == HAL_OK ) ? ( ns1 + ns2 ) : 0;
}

int  DevSPI::send_recv( uint8_t ds, uint8_t *dd, int nd )
{
  return send_recv( &ds, 1, dd, nd );
}


// send 1 bytes more ?
int  DevSPI::duplex( const uint8_t *ds, uint8_t *dd, int nd )
{
  if( ds == nullptr || dd == nullptr || nd == 0 ) {
    return 0;
  }

  nss_pre_cond();
  HAL_StatusTypeDef rc = HAL_SPI_TransmitReceive( spi, (uint8_t*)ds, dd, nd, maxWait );
  if( rc == HAL_OK ) {
    rc = HAL_SPI_Receive( spi, (uint8_t*)(dd), nd, maxWait );
  }
  nss_post_cond();

  return ( rc == HAL_OK ) ? nd : 0;
}


int  DevSPI::sendSame( uint8_t ds, int ns )
{
  if( ns < 1  ||  spi->State != HAL_SPI_STATE_READY ) {
    return 0;
  }
  __HAL_LOCK( spi );

  spi->State       = HAL_SPI_STATE_BUSY_TX;
  spi->ErrorCode   = HAL_SPI_ERROR_NONE;

  if( ( spi->Instance->CR1 & SPI_CR1_SPE ) != SPI_CR1_SPE ) { // need to reenable?
    __HAL_SPI_ENABLE( spi );
  }

  int n = 0;
  for( n=0; n<ns; ++n )  {
    if( waitForFlag( SPI_FLAG_TXE, SPI_FLAG_TXE ) != HAL_OK )  {
      return 0;
    }
    *( (__IO uint8_t*)&spi->Instance->DR ) = ds;
  }

  /* Check the end of the transaction */
  if( waitForFlag( SPI_FLAG_BSY, 0 ) != HAL_OK ) {
    // TODO: error
    return 0;
  }

  __HAL_SPI_CLEAR_OVRFLAG( spi );

  spi->State = HAL_SPI_STATE_READY;

  __HAL_UNLOCK( spi );

  return n;
}

int  DevSPI::txrx( uint8_t ds, uint8_t *dr )
{
  if( spi->State != HAL_SPI_STATE_READY ) {
    return 0;
  }
  uint8_t xx;
  if( !dr ) { dr = &xx; };
  __HAL_LOCK( spi );

  spi->State       = HAL_SPI_STATE_BUSY_TX;
  spi->ErrorCode   = HAL_SPI_ERROR_NONE;

  if( waitForFlag( SPI_FLAG_TXE, SPI_FLAG_TXE ) != HAL_OK )  {
    // spi->ErrorCode   = HAL_TIMEOUT;
    return 0;
  }
  *( (__IO uint8_t*)&spi->Instance->DR ) = ds;

  spi->State       = HAL_SPI_STATE_BUSY_RX;
  if( waitForFlag( SPI_FLAG_RXNE, SPI_FLAG_RXNE ) != HAL_OK )  {
    // spi->ErrorCode   = HAL_SPI_ERROR_FRE;
    return 0;
  }
  *dr = *( (__IO uint8_t*)&spi->Instance->DR );

  spi->State = HAL_SPI_STATE_READY;

  __HAL_UNLOCK( spi );

  return 1;
}
