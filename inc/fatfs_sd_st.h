#ifndef _FATFS_SD_ST_H
#define _FATFS_SD_ST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ff.h>
#include <sd_diskio.h>

extern uint8_t retSD;   // global return value
extern char SD_Path[8]; //* SD logical drive path

void MX_FATFS_SD_Init(void);

#ifdef __cplusplus
}
#endif
#endif // _FATFS_SD_ST_H

