#ifndef __USBD_CONF_H
#define __USBD_CONF_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>

#include <oxc_archdef.h>
// this includes need only for debug! here USBD_DEBUG_LEVEL = 0
// #include <stdio.h>
// #include <string.h>

// 2.5.3
#define USBD_OXC_VERSION 205030

#define DEVICE_FS               0
#define DEVICE_HS               1


/* Common Config */
#define USBD_MAX_NUM_INTERFACES               1
#define USBD_MAX_NUM_CONFIGURATION            1
#define USBD_MAX_STR_DESC_SIZ                 0x200
#define USBD_SUPPORT_USER_STRING              0
#define USBD_SELF_POWERED                     1
#define USBD_LPM_ENABLED                      0
#define USBD_CLASS_BOS_ENABLED                0
#define USBD_DEBUG_LEVEL                      0

/* Exported macro ------------------------------------------------------------*/
/* Memory management macros */

void *USBD_static_malloc(uint32_t size);
void USBD_static_free(void *p);

#define MAX_STATIC_ALLOC_SIZE     140 /*CDC Class Driver Structure size*/

#define USBD_malloc               (uint32_t *)USBD_static_malloc
#define USBD_free                 USBD_static_free
#define USBD_memset               /* Not used */
#define USBD_memcpy               /* Not used */

// #define USBD_malloc               malloc
// #define USBD_free                 free
// #define USBD_memset               memset
// // atu: memcpy seems to be buggy
// #define USBD_memcpy               memmove

/* DEBUG macros */
#if (USBD_DEBUG_LEVEL > 0)
#define  USBD_UsrLog(...)   printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)

#define  USBD_ErrLog(...)   printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define  USBD_DbgLog(...)   printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_DbgLog(...)
#endif

#endif /* __USBD_CONF_H */

