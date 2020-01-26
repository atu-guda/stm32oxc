#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"


/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;


/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
  if( USBD_Init( &hUsbDeviceFS, &VCP_Desc, DEVICE_FS ) != USBD_OK ) {
    // Error_Handler();
    return;
  }
  if( USBD_RegisterClass( &hUsbDeviceFS, &USBD_CDC ) != USBD_OK ) {
    // Error_Handler();
    return;
  }
  if( USBD_CDC_RegisterInterface( &hUsbDeviceFS, &USBD_Interface_fops_FS ) != USBD_OK ) {
    // Error_Handler();
    return;
  }
  if( USBD_Start( &hUsbDeviceFS ) != USBD_OK ) {
    // Error_Handler();
    return;
  }
}

