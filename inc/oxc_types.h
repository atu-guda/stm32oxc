#ifndef _OXC_TYPES_H
#define _OXC_TYPES_H

#include <stdint.h>

// types and defines for C-compatible part

#define UNUSED_ARG __attribute__((unused))

#if defined ( __GNUC__ )
  #ifndef __weak
    #define __weak   __attribute__((weak))
  #endif
  #ifndef __packed
    #define __packed __attribute__((__packed__))
  #endif
#endif

#define PORT_BITS 16
#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000

#define BIT0M  0x0001_mask
#define BIT1M  0x0002_mask
#define BIT2M  0x0004_mask
#define BIT3M  0x0008_mask
#define BIT4M  0x0010_mask
#define BIT5M  0x0020_mask
#define BIT6M  0x0040_mask
#define BIT7M  0x0080_mask
#define BIT8M  0x0100_mask
#define BIT9M  0x0200_mask
#define BIT10M 0x0400_mask
#define BIT11M 0x0800_mask
#define BIT12M 0x1000_mask
#define BIT13M 0x2000_mask
#define BIT14M 0x4000_mask
#define BIT15M 0x8000_mask

#define BAD_ADDR ((void*)(0xFFFFFFFF))
#ifndef NL
  #define NL "\n"
#endif

enum ReturnCode {
  rcOk = 0,
  rcInfo = 1,
  rcWarn = 2,
  rcExtra = 3,
  rcEnd = 4,
  rcErr = 10,
  rcFatal = 20
};

#ifndef   __IO
  #define __IO volatile
#endif
typedef __IO uint32_t reg32;
typedef const char *const ccstr;
typedef uint32_t mu_t; // mutex_t alike

typedef void (*AuxTickFun)(void);


#endif

