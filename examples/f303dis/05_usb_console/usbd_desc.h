#ifndef __USBD_DESC_H
#define __USBD_DESC_H

/* Includes ------------------------------------------------------------------*/
#include <usbd_def.h>

#define USBD_VID                      0x0483
#define USBD_PID                      0x5740
#define USBD_LANGID_STRING            0x409
#define USBD_MANUFACTURER_STRING      "Atu_labs"
#define USBD_PRODUCT_FS_STRING        "STM32 Virtual ComPort"
#define USBD_CONFIGURATION_FS_STRING  "VCP Config"
#define USBD_INTERFACE_FS_STRING      "VCP Interface"

/* Exported constants --------------------------------------------------------*/
#define         DEVICE_ID1          (0x1FFFF7AC)
#define         DEVICE_ID2          (0x1FFFF7B0)
#define         DEVICE_ID3          (0x1FFFF7B4)

#define  USB_SIZ_STRING_SERIAL       0x1A

extern USBD_DescriptorsTypeDef VCP_Desc;
extern USBD_HandleTypeDef USBD_Dev;

#endif /* __USBD_DESC_H */
