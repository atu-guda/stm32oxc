#ifndef _FATFS_H
#define _FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ff.h>
#include <ff_gen_drv.h>
#include <sd_diskio.h>

extern uint8_t retSD;   // global return value
extern char SD_Path[4]; //* SD logical drive path

void MX_FATFS_Init(void);

#ifdef __cplusplus
}
#endif
#endif // _FATFS_H

