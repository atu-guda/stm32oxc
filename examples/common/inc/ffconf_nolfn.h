//    FatFs - FAT file system configuration file  R0.11 (C)ChaN, 2015
//    mod by atu

#ifndef _FFCONF
#define _FFCONF 68300  /* Revision ID */

/*-----------------------------------------------------------------------------/
/ Additional user header to be used
/-----------------------------------------------------------------------------*/
// #include <stm32f4xx_hal.h>
#include <oxc_base.h>
#include <bsp_driver_sd.h>

#define _FS_READONLY         0      /* 0:Read/Write or 1:Read only */
#define _FS_MINIMIZE         0      /* 0 to 3 */
#define _USE_STRFUNC         1      /* 0:Disable or 1-2:Enable 1: w/o CR/LF conv*/
#define _USE_FIND            1
#define _USE_MKFS            0
#define _USE_FASTSEEK        1
#define _USE_EXPAND          0
#define _USE_CHMOD           0
#define _USE_LABEL           1
#define _USE_FORWARD         0
/*-----------------------------------------------------------------------------/
/ Locale and Namespace Configurations
/-----------------------------------------------------------------------------*/

#define _CODE_PAGE         866
//   1    - ASCII (No extended character. Valid for only non-LFN configuration.)

#define _USE_LFN     0    //* 0: no, 1: buf: static, 2: buf: stack, 3: buf: HEAP */
//#define _USE_LFN     0    //* 0: no, 1: buf: static, 2: buf: stack, 3: buf: HEAP */
#define _MAX_LFN     0  /* Maximum LFN length to handle (12 to 255) */
/* The _USE_LFN option switches the LFN feature.
/
/   0: Disable LFN feature. _MAX_LFN has no effect.
/   1: Enable LFN with static working buffer on the BSS. Always NOT thread-safe.
/   2: Enable LFN with dynamic working buffer on the STACK.
/   3: Enable LFN with dynamic working buffer on the HEAP.
/
/  When enable the LFN feature, Unicode handling functions (option/unicode.c) must
/  be added to the project. The LFN working buffer occupies (_MAX_LFN + 1) * 2 bytes.
/  When use stack for the working buffer, take care on stack overflow. When use heap
/  memory for the working buffer, memory management functions, ff_memalloc() and
/  ff_memfree(), must be added to the project. */

#define _LFN_UNICODE    0 /* 0:ANSI/OEM or 1:Unicode */
/* This option switches character encoding on the API. (0:ANSI/OEM or 1:Unicode)
/  To use Unicode string for the path name, enable LFN feature and set _LFN_UNICODE
/  to 1. This option also affects behavior of string I/O functions. */

#define _STRF_ENCODE    3
/* When _LFN_UNICODE is 1, this option selects the character encoding on the file to
/  be read/written via string I/O functions, f_gets(), f_putc(), f_puts and f_printf().
/
/  0: ANSI/OEM
/  1: UTF-16LE
/  2: UTF-16BE
/  3: UTF-8
/
/  When _LFN_UNICODE is 0, this option has no effect. */

#define _FS_RPATH       2 /* 0: no, 1: f_chdir, f_chdrive, 2: + f_getcwd */

/*---------------------------------------------------------------------------/
/ Drive/Volume Configurations
/----------------------------------------------------------------------------*/

#define _VOLUMES    4
/* Number of volumes (logical drives) to be used. */

/* USER CODE BEGIN Volumes */
#define _STR_VOLUME_ID  0  /* 0:Use only 0-9 for drive ID, 1:Use strings for drive ID */
#define _VOLUME_STRS   "RAM","NAND","CF","SD","SD2","USB","USB2","USB3"
/* _STR_VOLUME_ID option switches string volume ID feature.
/  When _STR_VOLUME_ID is set to 1, also pre-defined strings can be used as drive
/  number in the path name. _VOLUME_STRS defines the drive ID strings for each
/  logical drives. Number of items must be equal to _VOLUMES. Valid characters for
/  the drive ID strings are: A-Z and 0-9. */
/* USER CODE END Volumes */

#define _MULTI_PARTITION     1 /* 0:Single partition, 1:Multiple partition */
/* This option switches support of multi-partition on a physical drive.
/  By default (0), each logical drive number is bound to the same physical drive
/  number and only an FAT volume found on the physical drive will be mounted.
/  When multi-partition is enabled (1), each logical drive number can be bound to
/  arbitrary physical drive and partition listed in the VolToPart[]. Also f_fdisk()
/  funciton will be available. */

#define _MIN_SS    512  /* 512, 1024, 2048 or 4096 */
#define _MAX_SS    512  /* 512, 1024, 2048 or 4096 */
#define  _USE_TRIM      0

#define _FS_NOFSINFO    0 /* 0,1,2 or 3 */
/* If you need to know correct free space on the FAT32 volume, set bit 0 of this
/  option, and f_getfree() function at first time after volume mount will force
/  a full FAT scan. Bit 1 controls the use of last allocated cluster number.
/
/  bit0=0: Use free cluster count in the FSINFO if available.
/  bit0=1: Do not trust free cluster count in the FSINFO.
/  bit1=0: Use last allocated cluster number in the FSINFO if available.
/  bit1=1: Do not trust last allocated cluster number in the FSINFO.
*/

/*---------------------------------------------------------------------------/
/ System Configurations
/----------------------------------------------------------------------------*/

#define _FS_TINY             0      /* 0:Normal or 1:Tiny */
#define _FS_EXFAT  0
#define _FS_NORTC  1
#define _NORTC_MON  4
#define _NORTC_MDAY  30
#define _NORTC_YEAR  2018
/* The _FS_NORTC option switches timestamp feature. If the system does not have
/  an RTC function or valid timestamp is not needed, set _FS_NORTC to 1 to disable
/  the timestamp feature. All objects modified by FatFs will have a fixed timestamp
/  defined by _NORTC_MON, _NORTC_MDAY and _NORTC_YEAR.
/  When timestamp feature is enabled (_FS_NORTC  == 0), get_fattime() function need
/  to be added to the project to read current time form RTC. _NORTC_MON,
/  _NORTC_MDAY and _NORTC_YEAR have no effect.
/  These options have no effect at read-only configuration (_FS_READONLY == 1). */

#define _FS_LOCK    2     /* 0:Disable or >=1:Enable */
/* The _FS_LOCK option switches file lock feature to control duplicated file open
/  and illegal operation to open objects. This option must be 0 when _FS_READONLY
/  is 1.
/
/  0:  Disable file lock feature. To avoid volume corruption, application program
/      should avoid illegal open, remove and rename to the open objects.
/  >0: Enable file lock feature. The value defines how many files/sub-directories
/      can be opened simultaneously under file lock control. Note that the file
/      lock feature is independent of re-entrancy. */

#define _FS_REENTRANT    1  /* 0:Disable or 1:Enable */
#if _FS_REENTRANT
  #include "cmsis_os.h"
  #define _FS_TIMEOUT    1000 /* Timeout period in unit of time ticks */
  #define  _SYNC_t         QueueHandle_t
  // #define _SYNC_t          SemaphoreHandle_t
#endif
/* The option _FS_REENTRANT switches the re-entrancy (thread safe) of the FatFs
/  module itself. Note that regardless of this option, file access to different
/  volume is always re-entrant and volume control functions, f_mount(), f_mkfs()
/  and f_fdisk() function, are always not re-entrant. Only file/directory access
/  to the same volume is under control of this function.
/
/   0: Disable re-entrancy. _FS_TIMEOUT and _SYNC_t have no effect.
/   1: Enable re-entrancy. Also user provided synchronization handlers,
/      ff_req_grant(), ff_rel_grant(), ff_del_syncobj() and ff_cre_syncobj()
/      function, must be added to the project. Samples are available in
/      option/syscall.c.
/
/  The _FS_TIMEOUT defines timeout period in unit of time tick.
/  The _SYNC_t defines O/S dependent sync object type. e.g. HANDLE, ID, OS_EVENT*,
/  SemaphoreHandle_t and etc.. A header file for O/S definitions needs to be
/  included somewhere in the scope of ff.h. */

#if _USE_LFN == 3
#if !defined(ff_malloc) || !defined(ff_free)
#include <stdlib.h>
#endif

#if !defined(ff_malloc)
#define ff_malloc malloc
#endif

#if !defined(ff_free)
#define ff_free free
#endif
#endif

#endif /* _FFCONF */
