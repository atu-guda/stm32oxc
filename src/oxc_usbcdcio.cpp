#include <oxc_usbcdcio.h>

void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler( &hpcd );
}


int UsbcdcIO::sendBlockSync( const char *s, int l )
{
  if( !s || l < 1 ) {
    return 0;
  }
  USBD_CDC_SetTxBuffer( usb_dev, (uint8_t*)s, l );
  uint8_t rc;
  for( int n_try = 0; n_try < wait_tx; ++n_try ) {
    rc = USBD_CDC_TransmitPacket( usb_dev );
    if( rc == USBD_OK ) {
      return l;
    }
    if( rc != USBD_BUSY ) {
      err = rc;
      return 0;
    }
    delay_ms( 1 );
  }
  err = USBD_BUSY;
  return 0;
}

