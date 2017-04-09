#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SD_Path[4];  /* SD logical drive path */

/* USER CODE BEGIN VolToPart */
/* Volume - Partition resolution table should be user defined in case of Multiple partition */
/* When multi-partition feature is enabled (1), each logical drive number is bound to arbitrary physical drive and partition
listed in the VolToPart[] */
PARTITION VolToPart[4];

void MX_FATFS_Init(void)
{
  retSD = FATFS_LinkDriver(&SD_Driver, SD_Path);
}

