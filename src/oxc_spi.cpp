#include <oxc_spi.h>

void DevSPI::initSPI()
{
  __HAL_SPI_ENABLE( spi );
  if( nss_pin ) {
    nss_pin->initHW();
  }
  nss_post();
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
  nss_post();
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


int DevSPI::send( uint8_t ds )
{
  return send( &ds, 1 );
}

int  DevSPI::send( const uint8_t *ds, int ns )
{
  if( ds == nullptr || ns < 1 ) {
    return 0;
  }

  nss_pre();
  HAL_StatusTypeDef rc = HAL_SPI_Transmit( spi, (uint8_t*)ds, ns, maxWait );
  nss_post();

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

  nss_pre();
  HAL_StatusTypeDef rc = HAL_SPI_Receive( spi, (uint8_t*)(dd), nd, maxWait );
  nss_post();

  return ( rc == HAL_OK ) ? nd : 0;
}

int  DevSPI::send_recv( const uint8_t *ds, int ns, uint8_t *dd, int nd )
{
  if( ds == nullptr || dd == nullptr ) {
    return 0;
  }

  nss_pre();
  HAL_StatusTypeDef rc = HAL_SPI_Transmit( spi, (uint8_t*)ds, ns, maxWait );
  if( rc == HAL_OK ) {
    rc = HAL_SPI_Receive( spi, (uint8_t*)(dd), nd, maxWait );
  }
  nss_post();

  return ( rc == HAL_OK ) ? nd : 0;
}

int  DevSPI::send2( const uint8_t *ds1, int ns1, const uint8_t *ds2, int ns2 )
{
  if( ds1 == nullptr || ns1 < 1 || ds2 == nullptr || ns2 < 1 ) {
    return 0;
  }

  nss_pre();
  HAL_StatusTypeDef rc = HAL_SPI_Transmit( spi, (uint8_t*)ds1, ns1, maxWait );
  if( rc == HAL_OK ) {
    rc = HAL_SPI_Transmit( spi, (uint8_t*)(ds2), ns2, maxWait );
  }
  nss_post();

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

  nss_pre();
  HAL_StatusTypeDef rc = HAL_SPI_TransmitReceive( spi, (uint8_t*)ds, dd, nd, maxWait );
  if( rc == HAL_OK ) {
    rc = HAL_SPI_Receive( spi, (uint8_t*)(dd), nd, maxWait );
  }
  nss_post();

  return ( rc == HAL_OK ) ? nd : 0;
}




