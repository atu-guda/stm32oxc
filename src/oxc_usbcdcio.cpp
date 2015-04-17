#include <oxc_usbcdcio.h>

int UsbcdcIO::sendBlockSync( const char *s, int l )
{
  if( !s || l < 1 ) {
    return 0;
  }
  USBD_CDC_SetTxBuffer( usb_dev, (uint8_t*)s, l );
  USBD_CDC_TransmitPacket( usb_dev );
  return l;
}

