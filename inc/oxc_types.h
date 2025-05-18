#ifndef _OXC_TYPES_H
#define _OXC_TYPES_H

#include <stdint.h>

// types and deines for both C and C++

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

