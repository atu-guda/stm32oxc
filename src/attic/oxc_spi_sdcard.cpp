#include <oxc_spi_sdcard.h>

#define SD_DEBUG 1

#if SD_DEBUG
#include <oxc_debug1.h> // tmp for debug
#define prD(x) pr(x)
#else
#define prD(x) {}
#endif


uint8_t DevSPI_SdCard::crc7( uint8_t t, uint8_t d )
{
  t ^= d;
  for( uint8_t i=0; i<8; ++i ) {
    if( t & 0x80 ) {
      t ^= CRC7_INIT;
    }
    t <<= 1;
  }
  return t;
}

uint8_t DevSPI_SdCard::crc7( const uint8_t *p, uint16_t l )
{
  uint8_t crc = 0;
  for ( uint16_t j=0; j<l; ++j) {
    crc = crc7( crc, p[j] );
  }

  return ( crc >> 1 );
}

uint16_t DevSPI_SdCard::crc16_ccitt( uint16_t crc, uint8_t d )
{
  crc  = ((uint8_t)( crc >> 8 )) | (crc << 8);
  crc ^= d;
  crc ^= ((uint8_t)( crc & 0xff )) >> 4;
  crc ^= ( crc << 8 ) << 4;
  crc ^= ( (crc & 0xff) << 4 ) << 1;
  return crc;
}

uint16_t DevSPI_SdCard::crc16( const uint8_t *p, uint16_t l )
{
  uint16_t crc = 0;
  for( uint16_t i=0; i<l; ++i ) {
    crc = crc16_ccitt( crc, p[i] );
  }

  return crc;
}

int DevSPI_SdCard::sd_cmd( uint8_t c, uint32_t arg )
{
  #if SD_DEBUG
  pr( "CMD: " ); pr_d( c ); pr( " arg= " ); pr_d( arg ); pr( " = " ); pr_h( arg ); pr( NL );
  #endif
  uint8_t crc = 0;
  err_c = ERR_SPI; // beforehand
  uint8_t t = 0x40 | c;   if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = arg >> 24;          if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = arg >> 16;          if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = arg >>  8;          if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = arg;                if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = crc | 0x01;         if( !spi.txrx( t ) ) return 0;
  err_c = 0;
  return 6;
}

bool DevSPI_SdCard::getR1()
{
  r1 = 0xFF; err_c = 0;
  prD( "** R1= " );
  for( uint32_t i=0; i<n_try; ++i ) {
    if( !spi.txrx( 0xFF, &r1 ) ) {
      err_c = ERR_SPI; r1 = 0xFF;
      prD( " err: SPI" NL );
      return false;
    };
    if( ! ( r1 & 0x80 )  ) {
      #if SD_DEBUG
      pr_h( r1 );
      #endif
      return true;
    }
  }
  prD( " err: TOUT" NL );
  err_c = ERR_TOUT;
  return false;
}


bool DevSPI_SdCard::getR2()
{
  err_c = 0;
  r2 = 0xFFFF;
  uint8_t r1a;
  if( !getR1() ) { return false; };

  if( !spi.txrx( 0xFF, &r1a  ) ) {
    err_c = ERR_SPI;
    return false;
  }

  r2 =  (r1<<8) | r1a;
  return true;
}

bool DevSPI_SdCard::getR7( uint8_t bad_bits )
{
  if( !getR1() ) {
    return false;
  }
  if( r1 & bad_bits ) {
    err_c = ERR_BITS;
    return false;
  }

  uint8_t r0;
  spi.txrx( 0xFF, &r0 ); // TODO: more check;
  uint32_t r = r0 << 24;
  spi.txrx( 0xFF, &r0 );
  r |= r0 << 16;
  spi.txrx( 0xFF, &r0 );
  r |= r0 <<  8;
  spi.txrx( 0xFF, &r0 );
  r |= r0;
  r7 = r;
  return true;
}

int DevSPI_SdCard::sd_cmd_r1( uint8_t c, uint32_t arg, uint8_t bad_bits )
{
  spi.nss_pre();
  if( !sd_cmd( c, arg ) ) {
    return 0;
  }
  if( !getR1()  || r1 == 0xFF ) {
    return 0;
  }
  nec();
  spi.nss_post();
  #if SD_DEBUG
  pr( " R1: " ); pr_h( r1 ); pr( " bad: " ); pr_d( r1 & bad_bits ); pr( NL );
  #endif
  if( r1 & bad_bits ) {
    err_c = ERR_BITS;
    return 0;
  }
  return 1;
}

int DevSPI_SdCard::sd_cmd_r7( uint8_t c, uint32_t arg, uint8_t bad_bits )
{
  spi.nss_pre();
  if( !sd_cmd( c, arg ) ) {
    return 0;
  }
  if( !getR7( bad_bits ) ) {
    return 0;
  }
  nec();
  spi.nss_post();
  #if SD_DEBUG
  pr( " R1: " ); pr_h( r1 ); pr( " R7: "); pr_h( r7 ); pr( " bad: " ); pr_d( r1 & bad_bits ); pr( NL );
  #endif
  if( r1 & bad_bits ) {
    err_c = ERR_BITS;
    return 0;
  }
  return 1;
}



void DevSPI_SdCard::nec()
{
  spi.sendSame( 0xFF, 8 );
}

int DevSPI_SdCard::init()
{
  int tries = 1000;
  uint32_t hcs = 0;
  caps = 0; inited = false;

  //   /* start with 100-400 kHz clock */
  //   // spi_set_speed(SD_SPEED_400KHZ);

  prD( "cmd0 - reset.. " );
  spi.nss_post();/* 74+ clocks with CS high */
  spi.sendSame( 0xFF, 10 );
  /* reset */
  if( !sd_cmd_r1( 0, 0 ) ) {
    return 1;
  }

  prD( "success idle" NL );

  prD( "CMD8 - voltage.. " NL );

  /* ask about voltage supply */
  if( !  sd_cmd_r7( 8, 0x1AA, ~( 0x01 | 0x04 ) ) ) {
    return 8;
  }

  caps |= CAP_V2_00;

  if( r1 == 0x01 ) {
    prD( "success, SD v2.x" NL );
  } else if ( r1 & 0x4 ) {
    caps &= ~CAP_V2_00;
    prD( "not implemented, SD v1.x" NL );
  } else {
    prD( "fail_8" NL );
    return 3;
  }


  prD( "cmd58 - ocr.. " NL );
  /* ask about voltage supply */
  if( !  sd_cmd_r7( 58, 0x1AA, ~( 0x01 | 0x04 ) ) ) {
    return 58;
  }

  for ( int i=4; i<24; ++i ) {
    if( r7 & (1<<i) ) {
      break;
    }
    #if SD_DEBUG
    pr( "Vdd voltage window: "); pr_d( (12+i)/10 );  pr("."); pr_d( (12+i)%10 ); pr( NL );
    #endif
  }

  for( int i=23; i>=4; ++i ) {
    if ( r7 & 1<<i ) {
      break;
    }
    // CCS shouldn't be valid here yet
    #if SD_DEBUG
    pr_d( (13+i)/10 ); pr( "." );  pr_d( (13+i)%10 ); pr( "V, CSS: ");
    pr_d( r7>>30 & 1); pr( " status: " ); pr_d( r7>>31 ); pr( NL );
    #endif
  }
  prD( "success_58" NL);


  prD("acmd41 - hcs.. ");

  if( caps & CAP_V2_00 ) {// say we support SDHC
    hcs = 1<<30;
  }

  /* needs to be polled until in_idle_state becomes 0 */
  do {
    /* send we don't support SDHC */
    if( !sd_cmd_r1( 55, 0 ) ) {
      return 55;
    }

    if( !sd_cmd_r1( 41, hcs ) ) {
      return 41;
    }

  } while( r1 != 0 && tries-- );

  if( tries == -1 ) {
    prD("timeouted" NL);
    return 100;
  }
  prD("success_55_41" NL);

  // Seems after this card is initialized which means bit 0 of R1
  //  will be cleared. Not too sure.

  if( caps & CAP_V2_00 ) {
    prD( "cmd58 - ocr, 2nd time.. " NL );
    /* ask about voltage supply */
    if( !  sd_cmd_r7( 58, 0x1AA, ~( 0x01 | 0x04 ) ) ) {
      return 58;
    }
    for( int i=4; i<24; ++i ) {
      if( r7 & 1<<i ) {
        break;
      }
      #if SD_DEBUG
      pr( "Vdd voltage window: "); pr_d( (12+i)/10 );  pr("."); pr_d( (12+i)%10 ); pr( NL );
      #endif
    }
    for( int i=23; i>=4; i-- ) {
      if( r7 & 1<<i ) {
        break;
      }
      // CCS shouldn't be valid here yet
      #if SD_DEBUG
      pr_d( (13+i)/10 ); pr( "." );  pr_d( (13+i)%10 ); pr( "V, CSS: ");
      pr_d( r7>>30 & 1); pr( " status: " ); pr_d( r7>>31 ); pr( NL );
      #endif
    }
    // XXX power up status should be 1 here, since we're finished initializing, but it's not. WHY?
    // that means CCS is invalid, so we'll set CAP_SDHC later
    if( r7>>30 & 1 ) {
      caps |= CAP_SDHC;
      prD( "set CAP_SDHS" NL );
    }

    prD( "success_58_x2" NL );
  }


  /* with SDHC block length is fixed to 1024 */
  if( (caps & CAP_SDHC) == 0) {
    prD("not SDHC: use cmd16 - block length.. ");
    if( !sd_cmd_r1( 16, 512 ) ) {
      return 16;
    }
    prD( "success_16" NL );
  }


  prD("cmd59 - enable crc.. ");
  /* crc on */
  if( !sd_cmd_r1( 59, 0 ) ) {
    return 59;
  }
  prD( "success_59" NL );


  //   /* now we can up the clock to <= 25 MHz */
  //   spi_set_speed(SD_SPEED_25MHZ);
  //
  inited = true;
  return 0;
}

#define TXRX( sd, rd ) \
  if( ! spi.txrx( sd, rd ) ) { \
      err_c = ERR_SPI;  return 0; \
  }


int DevSPI_SdCard::getData( uint8_t *d, int l )
{
  if( !d || !l || !inited ) {
    return 0;
  }

  err_c = 0;
  int tries = 20000;
  uint8_t r;
  uint16_t c16;
  uint16_t calc_crc;

  while( tries-- ) {
    TXRX( 0xFF, &r );
    if( r == 0xFE ) {
      break;
    }
  }
  if( tries < 0 ) {
    err_c = ERR_TOUT;  return 0;
  }

  int n = 0;
  for( n=0; n<l; ++n ) {
    TXRX( 0xFF, &r );
    d[n] = r;
  }

  TXRX( 0xFF, &r );
  c16  = r << 8;
  TXRX( 0xFF, &r );
  c16 |= r;

  calc_crc = crc16( d, l );
  if( c16 != calc_crc ) {
    err_c = ERR_CRC; return 0;
  }
  return n;
}

int DevSPI_SdCard::putData( const uint8_t *d, int l )
{
  if( !d || !l || !inited ) {
    return 0;
  }

  err_c = 0;
  uint8_t r;

  TXRX( 0xFE, &r ); // data start

  int n;
  for( n=0; n<l; ++n ) {
    r = d[n];
    TXRX( r, nullptr );
  }

  uint16_t crc = crc16( d, l );
  TXRX( crc>>8, nullptr );
  TXRX( crc, nullptr );

  // normally just one dummy read in between... specs don't say how many
  int tries = 10;
  while( tries-- ) {
    TXRX( 0xFF, &r );
    if( r != 0xFF ) {
      break;
    }
  }
  if( tries < 0 ) {
    err_c = ERR_TOUT;  return 0;
  }

  /* poll busy, about 300 reads for 256 MB card */
  tries = 100000;
  while( tries-- ) {
    TXRX( 0xFF, &r );
    if( r == 0xFF ) {
      break;
    }
  }
  if( tries < 0 ) {
    err_c = ERR_TOUT;  return 0;
  }

  /* data accepted, WIN */
  if( (r & 0x1f) == 0x05 ) {
    return n;
  }

  err_c = ERR_CRC;
  return 0;
}


