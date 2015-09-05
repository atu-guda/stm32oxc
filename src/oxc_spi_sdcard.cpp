#include <oxc_spi_sdcard.h>
#include <oxc_debug1.h> // tmp for debug


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
  uint8_t crc = 0;
  uint8_t t = 0x40 | c;   if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = arg >> 24;          if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = arg >> 16;          if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = arg >>  8;          if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = arg;                if( !spi.txrx( t ) ) return 0;  crc = crc7( crc, t );
  t = crc | 0x01;         if( !spi.txrx( t ) ) return 0;
  return 6;
}

uint8_t DevSPI_SdCard::getR1()
{
  uint8_t r;
  for( uint32_t i=0; i<n_try; ++i ) {
    if( !spi.txrx( 0xFF, &r ) ) { return 0xFF; };
    if( ! ( r & 0x80 )  ) {
      return r;
    }
  }
  return 0xFF;
}


uint16_t DevSPI_SdCard::getR2()
{
  uint8_t r1, r2;
  bool found = false;

  for( uint32_t i=0; i<n_try; ++i ) {
    spi.txrx( 0xFF, &r1 );
    if( ! ( r1 & 0x80 ) ) {
      found = true; break;
    }
  }
  if( !found ) {
    return 0xff;
  }
  spi.txrx( 0xFF, &r2  );

  return  (r1<<8) | r2;
}

uint8_t DevSPI_SdCard::getR7( uint32_t *r7 )
{
  uint8_t r1 = getR1();
  if( r1 != 0x01 ) {
    return r1;
  }

  uint8_t r0;
  spi.txrx( 0xFF, &r0 );
  uint32_t r = r0 << 24;
  spi.txrx( 0xFF, &r0 );
  r |= r0 << 16;
  spi.txrx( 0xFF, &r0 );
  r |= r0 <<  8;
  spi.txrx( 0xFF, &r0 );
  r |= r0;
  *r7 = r;
  return 0x01;
}

void DevSPI_SdCard::nec()
{
  spi.sendSame( 0xFF, 8 );
}

int DevSPI_SdCard::init()
{
  // int i;
  __label__ err_spi, err;
  int r;
  // uint32_t r7;
  // uint32_t r3;
  // int tries;
  //
  caps = 0;

//   /* start with 100-400 kHz clock */
//   // spi_set_speed(SD_SPEED_400KHZ);
//
  pr("cmd0 - reset.. ");
  spi.nss_post();/* 74+ clocks with CS high */
  spi.sendSame( 0xFF, 10 );
  /* reset */
  spi.nss_pre();
  sd_cmd( 0, 0 );
  r = getR1();
  nec();
  spi.nss_post();
  pr_shx( r );

  if( r == 0xff ) {
    goto err_spi;
  }
  if( r != 0x01 ) {
    pr("fail\n");
    goto err;
  }
  pr( "success idle" NL );
//
//
//   pr("cmd8 - voltage.. ");
//   /* ask about voltage supply */
//   spi.nss_pre();
//   sd_cmd(8, 0x1aa /* VHS = 1 */);
//   r = sd_get_r7(&r7);
//   sd_nec();
//   spi.nss_post();
//   hw->capabilities |= CAP_VER2_00;
//   if (r == 0xff)
//     goto err_spi;
//   if (r == 0x01)
//     pr("success, SD v2.x\n");
//   else if (r & 0x4) {
//     hw->capabilities &= ~CAP_VER2_00;
//     pr("not implemented, SD v1.x\n");
//   } else {
//     pr("fail\n");
//     print_r1(r);
//     return -2;
//   }
//
//
//   pr("cmd58 - ocr.. ");
//   /* ask about voltage supply */
//   spi.nss_pre();
//   sd_cmd(58, 0);
//   r = sd_get_r3(&r3);
//   sd_nec();
//   spi.nss_post();
//   if (r == 0xff)
//     goto err_spi;
//   if (r != 0x01 && !(r & 0x4)) { /* allow it to not be implemented - old cards */
//     pr("fail\n");
//     print_r1(r);
//     return -2;
//   }
//   else {
//     int i;
//     for (i=4; i<=23; i++)
//       if (r3 & 1<<i)
//         break;
//     pr("Vdd voltage window: %i.%i-", (12+i)/10, (12+i)%10);
//     for (i=23; i>=4; i--)
//       if (r3 & 1<<i)
//         break;
//     /* CCS shouldn't be valid here yet */
//     pr("%i.%iV, CCS:%li, power up status:%li\n",
//         (13+i)/10, (13+i)%10,
//         r3>>30 & 1, r3>>31);
//     pr("success\n");
//   }
//
//
//   pr("acmd41 - hcs.. ");
//   tries = 1000;
//   uint32_t hcs = 0;
//   /* say we support SDHC */
//   if (hw->capabilities & CAP_VER2_00)
//     hcs = 1<<30;
//
//   /* needs to be polled until in_idle_state becomes 0 */
//   do {
//     /* send we don't support SDHC */
//     spi.nss_pre();
//     /* next cmd is ACMD */
//     sd_cmd(55, 0);
//     r = sd_get_r1();
//     sd_nec();
//     spi.nss_post();
//     if (r == 0xff)
//       goto err_spi;
//     /* well... it's probably not idle here, but specs aren't clear */
//     if (r & 0xfe) {
//       pr("fail\n");
//       print_r1(r);
//       goto err;
//     }
//
//     spi.nss_pre();
//     sd_cmd(41, hcs);
//     r = sd_get_r1();
//     sd_nec();
//     spi.nss_post();
//     if (r == 0xff)
//       goto err_spi;
//     if (r & 0xfe) {
//       pr("fail\n");
//       print_r1(r);
//       goto err;
//     }
//   } while (r != 0 && tries--);
//   if (tries == -1) {
//     pr("timeouted\n");
//     goto err;
//   }
//   pr("success\n");
//
//   /* Seems after this card is initialized which means bit 0 of R1
//    * will be cleared. Not too sure. */
//
//
//   if (hw->capabilities & CAP_VER2_00) {
//     pr("cmd58 - ocr, 2nd time.. ");
//     /* ask about voltage supply */
//     spi.nss_pre();
//     sd_cmd(58, 0);
//     r = sd_get_r3(&r3);
//     sd_nec();
//     spi.nss_post();
//     if (r == 0xff)
//       goto err_spi;
//     if (r & 0xfe) {
//       pr("fail\n");
//       print_r1(r);
//       return -2;
//     }
//     else {
// #if 1
//       int i;
//       for (i=4; i<=23; i++)
//         if (r3 & 1<<i)
//           break;
//       pr("Vdd voltage window: %i.%i-", (12+i)/10, (12+i)%10);
//       for (i=23; i>=4; i--)
//         if (r3 & 1<<i)
//           break;
//       /* CCS shouldn't be valid here yet */
//       pr("%i.%iV, CCS:%li, power up status:%li\n",
//           (13+i)/10, (13+i)%10,
//           r3>>30 & 1, r3>>31);
//       // XXX power up status should be 1 here, since we're finished initializing, but it's not. WHY?
//       // that means CCS is invalid, so we'll set CAP_SDHC later
// #endif
//       if (r3>>30 & 1) {
//         hw->capabilities |= CAP_SDHC;
//       }
//
//       pr("success\n");
//     }
//   }
//
//
//   /* with SDHC block length is fixed to 1024 */
//   if ((hw->capabilities & CAP_SDHC) == 0) {
//     pr("cmd16 - block length.. ");
//     spi.nss_pre();
//     sd_cmd(16, 512);
//     r = sd_get_r1();
//     sd_nec();
//     spi.nss_post();
//     if (r == 0xff)
//       goto err_spi;
//     if (r & 0xfe) {
//       pr("fail\n");
//       print_r1(r);
//       goto err;
//     }
//     pr("success\n");
//   }
//
//
//   pr("cmd59 - enable crc.. ");
//   /* crc on */
//   spi.nss_pre();
//   sd_cmd(59, 0);
//   r = sd_get_r1();
//   sd_nec();
//   spi.nss_post();
//   if (r == 0xff)
//     goto err_spi;
//   if (r & 0xfe) {
//     pr("fail\n");
//     print_r1(r);
//     goto err;
//   }
//   pr("success\n");
//
//
//   /* now we can up the clock to <= 25 MHz */
//   spi_set_speed(SD_SPEED_25MHZ);
//
  return 0;

  err_spi:
  pr( "fail spi" NL);
  return 1;

  err:
  pr( "fail_2" NL);
  return 2;
}
