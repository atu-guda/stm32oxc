#include <oxc_usbcdcio.h>

#include <oxc_gpio.h> // debug

USBD_HandleTypeDef* UsbcdcIO::pusb_dev = nullptr;
UsbcdcIO* UsbcdcIO::static_usbcdcio = nullptr;

void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler( &hpcd );
}

void UsbcdcIO::init()
{
  USBD_Init( &usb_dev, &VCP_Desc, 0 );
  USBD_RegisterClass( &usb_dev, USBD_CDC_CLASS );
  USBD_CDC_RegisterInterface( &usb_dev, &cdc_fops );
  USBD_Start( &usb_dev );
}

int UsbcdcIO::sendBlockSync( const char *s, int l )
{
  if( !s || l < 1 ) {
    return 0;
  }
  USBD_CDC_SetTxBuffer( &usb_dev, (uint8_t*)s, l );
  uint8_t rc;
  for( int n_try = 0; n_try < wait_tx; ++n_try ) {
    rc = USBD_CDC_TransmitPacket( &usb_dev );
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

int8_t UsbcdcIO::CDC_Itf_Init()
{
  USBD_CDC_SetTxBuffer( pusb_dev, (uint8_t*)static_usbcdcio->tx_buf, 0 );
  USBD_CDC_SetRxBuffer( pusb_dev, (uint8_t*)static_usbcdcio->rx_buf );
  return USBD_OK;
}

int8_t UsbcdcIO::CDC_Itf_DeInit()
{
  return USBD_OK;
}

int8_t UsbcdcIO::CDC_Itf_Control ( uint8_t cmd, uint8_t* pbuf, uint16_t length UNUSED_ARG )
{
  switch (cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:
      /* Add your code here */
      break;

    case CDC_GET_ENCAPSULATED_RESPONSE:
      /* Add your code here */
      break;

    case CDC_SET_COMM_FEATURE:
      /* Add your code here */
      break;

    case CDC_GET_COMM_FEATURE:
      /* Add your code here */
      break;

    case CDC_CLEAR_COMM_FEATURE:
      /* Add your code here */
      break;

    case CDC_SET_LINE_CODING:
      static_usbcdcio->lineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
          (pbuf[2] << 16) | (pbuf[3] << 24));
      static_usbcdcio->lineCoding.format     = pbuf[4];
      static_usbcdcio->lineCoding.paritytype = pbuf[5];
      static_usbcdcio->lineCoding.datatype   = pbuf[6];

      /* Set the new configuration */
      // ComPort_Config();
      break;

    case CDC_GET_LINE_CODING:
      pbuf[0] = (uint8_t)(static_usbcdcio->lineCoding.bitrate);
      pbuf[1] = (uint8_t)(static_usbcdcio->lineCoding.bitrate >> 8);
      pbuf[2] = (uint8_t)(static_usbcdcio->lineCoding.bitrate >> 16);
      pbuf[3] = (uint8_t)(static_usbcdcio->lineCoding.bitrate >> 24);
      pbuf[4] = static_usbcdcio->lineCoding.format;
      pbuf[5] = static_usbcdcio->lineCoding.paritytype;
      pbuf[6] = static_usbcdcio->lineCoding.datatype;
      break;

    case CDC_SET_CONTROL_LINE_STATE:
      /* Add your code here */
      break;

    case CDC_SEND_BREAK:
      /* Add your code here */
      break;

    default:
      break;
  }

  return USBD_OK;
}

int8_t UsbcdcIO::CDC_Itf_Receive( uint8_t* Buf UNUSED_ARG , uint32_t *Len UNUSED_ARG )
{
  leds.toggle( 0x08 );

  USBD_CDC_ReceivePacket( pusb_dev ); // ???
  return USBD_OK;
}

