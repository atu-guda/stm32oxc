#include <oxc_base.h>

void default_USBFS_MspInit(void);

void HAL_PCD_MspInit( PCD_HandleTypeDef *hpcd UNUSED_ARG )
{
  default_USBFS_MspInit();
}
