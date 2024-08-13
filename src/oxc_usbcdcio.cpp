#include <errno.h>
#include <oxc_usbcdcio.h>

#include <oxc_gpio.h> // debug

USBD_HandleTypeDef* UsbcdcIO::pusb_dev = nullptr;
UsbcdcIO* UsbcdcIO::static_usbcdcio = nullptr;

int UsbcdcIO::init()
{
  if( USBD_Init( &usb_dev, &VCP_Desc, BOARD_USB_DEFAULT_TYPE ) != USBD_OK ) { // 0 = DEVICE_FS 1 = DEVICE_HS
    errno = 2000;
    return 0;
  }
  if( USBD_RegisterClass( &usb_dev, USBD_CDC_CLASS ) != USBD_OK ) {
    errno = 2001;
    return 0;
  }
  if( USBD_CDC_RegisterInterface( &usb_dev, &cdc_fops ) != USBD_OK ) {
    errno = 2002;
    return 0;
  }
  if( USBD_Start( &usb_dev ) != USBD_OK ) {
    errno = 2003;
    return 0;
  }
  return 1;
}

int UsbcdcIO::write_s( const char *s, int l )
{
  if( !s || l < 1 ) {
    return 0;
  }

  if( pusb_dev == nullptr && pusb_dev->dev_state != USBD_STATE_CONFIGURED ) {
    errno = EIO;
    return -1;
  }

  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)usb_dev.pClassData;
  if( !hcdc ) {
    return 0;
  }

  uint8_t rc = 1;
  for( int n_try = 0; n_try < 2*wait_tx; ++n_try ) {
    if( ! hcdc->TxState && ! on_transmit ) {
      rc = 0;
      break;
    }
    delay_bad_mcs( 50 );
  }

  if( rc != 0 ) {
    err = USBD_BUSY;
    return -1;
  }

  on_transmit = true;

  USBD_CDC_SetTxBuffer( &usb_dev, (uint8_t*)s, l );
  for( int n_try = 0; n_try < wait_tx; ++n_try ) {
    rc = USBD_CDC_TransmitPacket( &usb_dev );
    if( rc == USBD_OK ) {
      break;
    }
    if( rc != USBD_BUSY ) {
      err = rc;  errno = EIO;
      break;
    }
    delay_bad_mcs( 100 );
    rc = USBD_BUSY;
  }

  if( rc != USBD_OK ) {
    l = -1; err = rc;
  }

  for( int n_try = 0; n_try < 2*wait_tx; ++n_try ) {
    if( ! hcdc->TxState  && ! on_transmit ) {
      break;
    }
    delay_bad_mcs( 50 );
  }

  on_transmit = false;
  return l;
}

int8_t UsbcdcIO::CDC_Itf_Init()
{
  if( pusb_dev ) {
    USBD_CDC_SetTxBuffer( pusb_dev, (uint8_t*)static_usbcdcio->tx_buf, 0 );
    USBD_CDC_SetRxBuffer( pusb_dev, (uint8_t*)static_usbcdcio->rx_buf );
    return USBD_OK;
  }
  return USBD_BUSY;
}

int8_t UsbcdcIO::CDC_Itf_DeInit()
{
  return USBD_OK;
}

int8_t UsbcdcIO::CDC_Itf_Control( uint8_t cmd, uint8_t* pbuf, uint16_t length UNUSED_ARG )
{
  switch( cmd ) {
    case CDC_SEND_ENCAPSULATED_COMMAND:
      // NOP
      break;

    case CDC_GET_ENCAPSULATED_RESPONSE:
      // NOP
      break;

    case CDC_SET_COMM_FEATURE:
      // NOP
      /* Add your code here */
      break;

    case CDC_GET_COMM_FEATURE:
      // NOP
      break;

    case CDC_CLEAR_COMM_FEATURE:
      // NOP
      break;

    case CDC_SET_LINE_CODING:
      if( ! static_usbcdcio ) {
        break;
      }
      // leds.toggle( BIT1 );
      static_usbcdcio->lineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
          (pbuf[2] << 16) | (pbuf[3] << 24));
      static_usbcdcio->lineCoding.format     = pbuf[4];
      static_usbcdcio->lineCoding.paritytype = pbuf[5];
      static_usbcdcio->lineCoding.datatype   = pbuf[6];

      break;

    case CDC_GET_LINE_CODING:
      if( ! static_usbcdcio ) {
        break;
      }
      pbuf[0] = (uint8_t)( static_usbcdcio->lineCoding.bitrate       );
      pbuf[1] = (uint8_t)( static_usbcdcio->lineCoding.bitrate >>  8 );
      pbuf[2] = (uint8_t)( static_usbcdcio->lineCoding.bitrate >> 16 );
      pbuf[3] = (uint8_t)( static_usbcdcio->lineCoding.bitrate >> 24 );
      pbuf[4] = static_usbcdcio->lineCoding.format;
      pbuf[5] = static_usbcdcio->lineCoding.paritytype;
      pbuf[6] = static_usbcdcio->lineCoding.datatype;
      break;

    case CDC_SET_CONTROL_LINE_STATE:
      // leds.toggle( BIT2 );
      if( ! static_usbcdcio ) {
        break;
      }

      {
        // TODO: may be callback here
        USBD_SetupReqTypedef *req = (USBD_SetupReqTypedef *)pbuf;
        if( ( req->wValue & 0x0001 ) != 0 )  { // check DTR
          static_usbcdcio->ready_transmit = true;
          // leds.set( BIT1 );
        } else {
          static_usbcdcio->ready_transmit = false;
          // leds.reset( BIT1 );
        }
      }

      break;

    case CDC_SEND_BREAK:
      // leds.toggle( BIT1 );
      if( ! static_usbcdcio ) {
        break;
      }
      if( static_usbcdcio->handle_cbreak ) {
        break_flag = 1;
        if( static_usbcdcio->onSigInt ) {
          static_usbcdcio->onSigInt( '\x03' );
        }
      }
      break;

    default:
      break;
  }

  return USBD_OK;
}

int8_t UsbcdcIO::CDC_Itf_Receive( uint8_t* Buf, uint32_t *Len )
{
  if( static_usbcdcio && Buf && Len && *Len ) {
    static_usbcdcio->charsFromIrq( (char*)Buf, *Len );
  }

  USBD_CDC_ReceivePacket( pusb_dev ); // acq
  return USBD_OK;
}

int8_t UsbcdcIO::CDC_Itf_TransmitCplt( uint8_t * /*pbuf*/, uint32_t * /*Len*/, uint8_t /*epnum*/ )
{
  if( static_usbcdcio ) {
    static_usbcdcio->on_transmit = false;
  }
  return USBD_OK;
}
