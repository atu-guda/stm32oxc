#include <oxc_spi.h>

#ifdef USE_OXC_DEBUG
#include <oxc_devio.h>
#include <oxc_outstream.h>
#endif

void DevSPI::initSPI()
{
  __HAL_SPI_ENABLE( spi );
  if( nss_pin ) {
    nss_pin->initHW();
  }
  nss_post_cond();
  last_rc = HAL_OK; last_err = 0;
}

void DevSPI::deInit()
{
  __HAL_SPI_DISABLE( spi );
  HAL_SPI_DeInit( spi );
  last_rc = HAL_OK; last_err = 0;
}

void DevSPI::resetDev()
{
  __HAL_SPI_DISABLE( spi );
  delay_ms( 1 );
  __HAL_SPI_ENABLE( spi );
  nss_post_cond();
  delay_ms( 1 );
  last_rc = HAL_OK;
}

void DevSPI::nss_pre()
{
  if( !nss_pin ) { return; };
  if( inv_nss ) {
    nss_pin->set();
  } else {
    nss_pin->reset();
  }
  delay_bad_100ns( tss_delay_100ns );
}

void DevSPI::nss_post()
{
  if( !nss_pin ) { return; };
  delay_bad_100ns( tss_delay_100ns );
  if( inv_nss ) {
    nss_pin->reset();
  } else {
    nss_pin->set();
  }
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
  last_rc = HAL_SPI_Transmit( spi, (uint8_t*)ds, ns, maxWait );
  nss_post_cond();

  if ( last_rc != HAL_OK ) {
    last_err = getErr();
    return 0;
  }

  return ns;
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
  last_rc = HAL_SPI_Receive( spi, (uint8_t*)(dd), nd, maxWait );
  nss_post_cond();

  if ( last_rc != HAL_OK ) {
    last_err = getErr();
    return 0;
  }

  return nd;
}

int  DevSPI::send_recv( const uint8_t *ds, int ns, uint8_t *dd, int nd )
{
  if( ds == nullptr || dd == nullptr ) {
    return 0;
  }

  nss_pre_cond();

  if( ns > 0 ) {
    last_rc = HAL_SPI_Transmit( spi, (uint8_t*)ds, ns, maxWait );
  } else {
    last_rc = HAL_OK;
  }

  if( last_rc == HAL_OK  &&  nd > 0 ) {
    last_rc = HAL_SPI_Receive( spi, (uint8_t*)(dd), nd, maxWait );
  }

  nss_post_cond();

  if ( last_rc != HAL_OK ) {
    last_err = getErr();
    return 0;
  }

  return nd;
}

int  DevSPI::send2( const uint8_t *ds1, int ns1, const uint8_t *ds2, int ns2 )
{
  if( ds1 == nullptr || ns1 < 1 || ds2 == nullptr || ns2 < 1 ) {
    return 0;
  }

  nss_pre_cond();
  last_rc = HAL_SPI_Transmit( spi, (uint8_t*)ds1, ns1, maxWait );
  if( last_rc == HAL_OK ) {
    last_rc = HAL_SPI_Transmit( spi, (uint8_t*)(ds2), ns2, maxWait );
  }
  nss_post_cond();

  if ( last_rc != HAL_OK ) {
    last_err = getErr();
    return 0;
  }

  return ns1 + ns2;
}

int  DevSPI::send_recv( uint8_t ds, uint8_t *dd, int nd )
{
  return send_recv( &ds, 1, dd, nd );
}


// send 1 bytes more ?
int  DevSPI::duplex( const uint8_t *ds, uint8_t *dd, int nd )
{
  if( ds == nullptr || dd == nullptr || nd < 1 ) {
    return 0;
  }

  nss_pre_cond();
  last_rc = HAL_SPI_TransmitReceive( spi, (uint8_t*)ds, dd, nd, maxWait );
  nss_post_cond();

  if ( last_rc != HAL_OK ) {
    last_err = getErr();
    return 0;
  }

  return nd;
}



#ifdef USE_OXC_DEBUG

// TODO: per MCU type
const BitNames SPI_CR1_bitnames[] {
  {  0, 1, "CHPA" },
  {  1, 1, "CPOL" },
  {  2, 1, "MSTR" },
  {  3, 3, "BRn" },
  {  6, 1, "SPE" },
  {  7, 1, "LSBFIRST" },
  {  8, 1, "SSI" },
  {  9, 1, "SSM" },
  { 10, 1, "RXONLY" },
  { 11, 1, "DFF" },
  { 12, 1, "CRCNEXT" },
  { 13, 1, "CRCEN" },
  { 14, 1, "BIDIOE" },
  { 15, 1, "BIDIMODE" },
  {  0, 0, nullptr }
};

const BitNames SPI_CR2_bitnames[] {
  {  0, 1, "RXDMAEN" },
  {  1, 1, "TXDMAEN" },
  {  2, 1, "SSOE" },
  {  4, 1, "FRF" },
  {  5, 1, "ERRIE" },
  {  6, 1, "RXNEIE" },
  {  7, 1, "TXEIE" },
  {  0, 0, nullptr }
};

const BitNames SPI_SR_bitnames[] {
  {  0, 1, "RXNE" },
  {  1, 1, "TXE" },
  {  2, 1, "CHSIDE" },
  {  3, 1, "UDR" },
  {  4, 1, "CRCERR" },
  {  5, 1, "MODF" },
  {  6, 1, "OVR" },
  {  7, 1, "BSY" },
  {  8, 1, "FRE" },
  {  0, 0, nullptr }
};


void print_SPI_info( SPI_TypeDef *spi )
{
  if( !spi ) { return; }

  std_out << "SPI: "  << HexInt( spi ) << " CR1= " << HexInt( spi->CR1 )  //  pr_bitnames( spi->CR1, SPI_CR1_bitnames );
     << " CR2= " << HexInt( spi->CR2 ) << BitsStr( spi->CR2, SPI_CR2_bitnames )
     << " SR= "  << HexInt( spi->SR )  << BitsStr( spi->SR,  SPI_SR_bitnames  ) << NL;
}

void DevSPI::pr_info() const
{
  print_SPI_info( spi->Instance );
  std_out << " error_code= " << int( spi->ErrorCode )
     << " last_err= "   << int( last_err )
     << " last_rc= "    << int( last_rc ) << NL;
}
#endif

