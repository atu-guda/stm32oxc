#include <fatfs_sd_st.h>

uint8_t retSD;    /* Return value for SD */
char SD_Path[8];  /* SD logical drive path */

/* USER CODE BEGIN VolToPart */
/* Volume - Partition resolution table should be user defined in case of Multiple partition */
/* When multi-partition feature is enabled (1), each logical drive number is bound to arbitrary physical drive and partition
listed in the VolToPart[] */

#if defined(FF_MULTI_PARTITION) && FF_MULTI_PARTITION != 0
PARTITION VolToPart[10];
#endif

void MX_FATFS_SD_Init(void)
{
  retSD = FATFS_LinkDriver( &SD_Driver, SD_Path );
}

