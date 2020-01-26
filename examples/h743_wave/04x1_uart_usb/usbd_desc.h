#ifndef _USBD_DESC_H_
#define _USBD_DESC_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <usbd_def.h>

#define USBD_VID                      0x0483
#define USBD_PID                      0x5740
#define USBD_LANGID_STRING            0x409
#define USBD_MANUFACTURER_STRING      "Atu_labs"
#define USBD_PRODUCT_FS_STRING        "STM32 Virtual ComPort"
#define USBD_CONFIGURATION_FS_STRING  "VCP Config"
#define USBD_INTERFACE_FS_STRING      "VCP Interface"

#define         DEVICE_ID1          (UID_BASE)
#define         DEVICE_ID2          (UID_BASE + 0x4)
#define         DEVICE_ID3          (UID_BASE + 0x8)

#define  USB_SIZ_STRING_SERIAL       0x1A

extern USBD_DescriptorsTypeDef VCP_Desc;
extern USBD_HandleTypeDef USBD_Dev;

#ifdef __cplusplus
}
#endif

#endif /* _USBD_DESC_H_ */

