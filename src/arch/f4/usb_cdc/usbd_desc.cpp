#include <usbd_core.h>
#include "usbd_desc.h"
#include <usbd_conf.h>

#ifndef USBD_VID
#define USBD_VID                      0x0483
#endif

#ifndef USBD_PID
#define USBD_PID                      0x5740
#endif

#ifndef USBD_LANGID_STRING
#define USBD_LANGID_STRING            0x409
#endif

#ifndef USBD_MANUFACTURER_STRING
#define USBD_MANUFACTURER_STRING      "Hidden manufacturer"
#endif

#ifndef USBD_PRODUCT_FS_STRING
#define USBD_PRODUCT_FS_STRING        "STM32 Virtual ComPort"
#endif

#ifndef USBD_CONFIGURATION_FS_STRING
#define USBD_CONFIGURATION_FS_STRING  "VCP Config"
#endif

#ifndef USBD_INTERFACE_FS_STRING
#define USBD_INTERFACE_FS_STRING      "VCP Interface"
#endif

static void Get_SerialNum(void);
static void IntToUnicode( uint32_t value, uint8_t *pbuf, uint8_t len );


uint8_t* USBD_CDC_DeviceDescriptor( USBD_SpeedTypeDef speed, uint16_t *length );
uint8_t* USBD_CDC_LangIDStrDescriptor( USBD_SpeedTypeDef speed, uint16_t *length );
uint8_t* USBD_CDC_ManufacturerStrDescriptor( USBD_SpeedTypeDef speed, uint16_t *length );
uint8_t* USBD_CDC_ProductStrDescriptor( USBD_SpeedTypeDef speed, uint16_t *length );
uint8_t* USBD_CDC_SerialStrDescriptor( USBD_SpeedTypeDef speed, uint16_t *length );
uint8_t* USBD_CDC_ConfigStrDescriptor( USBD_SpeedTypeDef speed, uint16_t *length );
uint8_t* USBD_CDC_InterfaceStrDescriptor( USBD_SpeedTypeDef speed, uint16_t *length );


USBD_DescriptorsTypeDef VCP_Desc = {
  USBD_CDC_DeviceDescriptor,
  USBD_CDC_LangIDStrDescriptor,
  USBD_CDC_ManufacturerStrDescriptor,
  USBD_CDC_ProductStrDescriptor,
  USBD_CDC_SerialStrDescriptor,
  USBD_CDC_ConfigStrDescriptor,
  USBD_CDC_InterfaceStrDescriptor
};

/** USB standard device descriptor. */
__ALIGN_BEGIN uint8_t USBD_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
  0x12,                       /* bLength */
  USB_DESC_TYPE_DEVICE,       /* bDescriptorType */
  0x00,                       /* bcdUSB */
  0x02,
  0x02,                       /* bDeviceClass */
  0x02,                       /* bDeviceSubClass */
  0x00,                       /* bDeviceProtocol */
  USB_MAX_EP0_SIZE,           /* bMaxPacketSize */
  LOBYTE( USBD_VID ),         /* idVendor */
  HIBYTE( USBD_VID ),         /* idVendor */
  LOBYTE( USBD_PID ),         /* idProduct */
  HIBYTE( USBD_PID ),         /* idProduct */
  0x00,                       /* bcdDevice rel. 2.00 */
  0x02,
  USBD_IDX_MFC_STR,           /* Index of manufacturer string */
  USBD_IDX_PRODUCT_STR,       /* Index of product string */
  USBD_IDX_SERIAL_STR,        /* Index of serial number string */
  USBD_MAX_NUM_CONFIGURATION  /* bNumConfigurations */
};

/** USB lang identifier descriptor. */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END = {
  USB_LEN_LANGID_STR_DESC,
  USB_DESC_TYPE_STRING,
  LOBYTE( USBD_LANGID_STRING ),
  HIBYTE( USBD_LANGID_STRING )
};

/* Internal string descriptor. */
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

__ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END = {
  USB_SIZ_STRING_SERIAL,
  USB_DESC_TYPE_STRING,
};


/**
  * @brief  Return the device descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t* USBD_CDC_DeviceDescriptor( USBD_SpeedTypeDef /* speed */, uint16_t *length )
{
  //
  *length = sizeof( USBD_DeviceDesc );
  return (uint8_t*)USBD_DeviceDesc;
}

/**
  * @brief  Return the LangID string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t* USBD_CDC_LangIDStrDescriptor( USBD_SpeedTypeDef /* speed */, uint16_t *length )
{
  //
  *length = sizeof( USBD_LangIDDesc );
  return (uint8_t*)USBD_LangIDDesc;
}

uint8_t* USBD_CDC_ProductStrDescriptor( USBD_SpeedTypeDef /* speed */, uint16_t *length )
{
  USBD_GetString( (uint8_t *)USBD_PRODUCT_FS_STRING, USBD_StrDesc, length );
  return USBD_StrDesc;
}

uint8_t* USBD_CDC_ManufacturerStrDescriptor( USBD_SpeedTypeDef /* speed */, uint16_t *length )
{
  //
  USBD_GetString( (uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length );
  return USBD_StrDesc;
}

uint8_t* USBD_CDC_SerialStrDescriptor( USBD_SpeedTypeDef /* speed */, uint16_t *length )
{
  *length = USB_SIZ_STRING_SERIAL;

  /* Update the serial number string descriptor with the data from the unique ID */
  Get_SerialNum();
  return (uint8_t*) USBD_StringSerial;
}

uint8_t* USBD_CDC_ConfigStrDescriptor( USBD_SpeedTypeDef /* speed */, uint16_t *length )
{
  USBD_GetString( (uint8_t *)USBD_CONFIGURATION_FS_STRING, USBD_StrDesc, length );
  return USBD_StrDesc;
}

uint8_t* USBD_CDC_InterfaceStrDescriptor( USBD_SpeedTypeDef /* speed */, uint16_t *length )
{
  USBD_GetString( (uint8_t *)USBD_INTERFACE_FS_STRING, USBD_StrDesc, length );
  return USBD_StrDesc;
}

static void Get_SerialNum(void)
{
  uint32_t deviceserial0, deviceserial1, deviceserial2;

  deviceserial0 = *(uint32_t*)DEVICE_ID1;
  deviceserial1 = *(uint32_t*)DEVICE_ID2;
  deviceserial2 = *(uint32_t*)DEVICE_ID3;

  deviceserial0 += deviceserial2;

  if( deviceserial0 != 0 ) {
    IntToUnicode( deviceserial0, &USBD_StringSerial[ 2], 8 );
    IntToUnicode( deviceserial1, &USBD_StringSerial[18], 4 );
  }
}

static void IntToUnicode( uint32_t value, uint8_t *pbuf, uint8_t len )
{
  for( uint8_t idx = 0; idx < len; idx++ ) {
    uint8_t v = (uint8_t)(value >> 28);
    pbuf[2 * idx] = ( v < 0x0A )  ?  ( v + '0' )  :  ( v  + 'A' - 10 );
    pbuf[2 * idx + 1] = 0;
    value <<= 4;
  }
}

