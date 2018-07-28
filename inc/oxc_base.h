#ifndef _OXC_BASE_H
#define _OXC_BASE_H

#include "board_cfg.h"

#if defined (STM32F0)
 #include <stm32f0xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BRR
 #define RESET_BIT_SHIFT 0
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_HIGH
 #define ADC_FREQ_MAX 14000000
#elif defined (STM32F1)
 #include <stm32f1xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BRR
 #define RESET_BIT_SHIFT 0
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_HIGH
 #define ADC_FREQ_MAX 14000000
#elif defined (STM32F2)
 #include <stm32f2xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BSRR
 #define RESET_BIT_SHIFT 16
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_VERY_HIGH
 #define ADC_FREQ_MAX 36000000
#elif defined (STM32F3)
 #include <stm32f3xx_hal.h>
 #include <Legacy/stm32_hal_legacy.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BRR
 #define RESET_BIT_SHIFT 0
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_HIGH
 #define ADC_FREQ_MAX 72000000
#elif defined (STM32F4)
 #include <stm32f4xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BSRR
 #define RESET_BIT_SHIFT 16
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_VERY_HIGH
 #define ADC_FREQ_MAX 36000000
#elif defined(STM32F7)
 #include <stm32f7xx_hal.h>
 #define SET_BIT_REG   BSRR
 #define RESET_BIT_REG BSRR
 #define RESET_BIT_SHIFT 16
 #define GPIO_SPEED_MAX GPIO_SPEED_FREQ_VERY_HIGH
 #define ADC_FREQ_MAX 36000000
#else
  #error "Unsupported MCU"
#endif

#if REQ_MCBASE != MCBASE
  #error "Required and given MCBASE is not equal"
#endif

#define UNUSED_ARG __attribute__((unused))

#if defined ( __GNUC__ )
  #ifndef __weak
    #define __weak   __attribute__((weak))
  #endif
  #ifndef __packed
    #define __packed __attribute__((__packed__))
  #endif
#endif

#include <oxc_irqlist.h>


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

extern int ready_to_start_scheduler;
extern int exit_rc;
extern volatile int dbg_val0, dbg_val1, dbg_val2, dbg_val3;
extern volatile int idle_flag;
extern volatile int break_flag;
extern char* __heap_top;

typedef __IO uint32_t reg32;
typedef const char *const ccstr;
#define BAD_ADDR ((void*)(0xFFFFFFFF))

#define ARR_SZ(x) (sizeof(x) / sizeof(x[0]))
#define ARR_AND_SZ(x) x, (sizeof(x) / sizeof(x[0]))

#ifndef NL
  #define NL "\r\n"
#endif

extern uint32_t delay_calibrate_value;

typedef uint32_t mu_t; // mutex_t alike

#ifdef __cplusplus
// template<typename T> class _ShowType; // to output deducted type
//                                       // _ShowType< decltype(XXXX) > xType;
 extern "C" {
#endif

inline void oxc_enable_interrupts(void)
{
  __asm__ volatile ( "CPSIE I\n" );
}

inline void oxc_disable_interrupts(void)
{
  __asm__ volatile ( "CPSID I\n" );
}


inline void oxc_dmb(void)
{
  __asm__ volatile ( "dmb" );
}

inline uint32_t oxc_ldrex( volatile uint32_t *addr )
{
  uint32_t rv;
  __asm__ volatile ( "ldrex %0, [%1]" : "=r" (rv) : "r" (addr) );
  return rv;
}

inline uint32_t oxc_strex( uint32_t val, volatile uint32_t *addr )
{
  uint32_t rv;
  __asm__ volatile ( "strex %0, %2, [%1]"
      : "=&r" (rv) : "r" (addr), "r" (val) );
  return rv;
}

void mu_lock( mu_t *m );
int  mu_trylock( mu_t *m ); // 0 - ok, like pthread
int  mu_waitlock( mu_t *m, uint32_t ms );
void mu_unlock( mu_t *m );

// void die4led( uint16_t n );
void taskYieldFun(void);
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
// misc functions
void _exit( int rc );
void die( uint16_t n );
void Error_Handler( int rc ); // defined at user program

void approx_delay_calibrate(void);
void do_delay_calibrate(void);
void delay_ms( uint32_t ms ); // base on vTaskDelay - switch to scheduler (if avail), or to HAL
int  delay_ms_brk( uint32_t ms ); // exit with 1 if break_flag is set;
int  delay_ms_until_brk( uint32_t *tc0, uint32_t ms );
void delay_mcs( uint32_t mcs );
// dumb delay functions - loop based - for use w/o timer and for small times
void delay_bad_n( uint32_t n );
void delay_bad_s( uint32_t s );
void delay_bad_ms( uint32_t ms );
void delay_bad_mcs( uint32_t mcs );

void SystemClock_Config(void);
int  SystemClockCfg(void); // returns: 0: ok >0 + set errno: error

// osfuncs, real or not
struct stat;
struct tms;
int _getpid(void);
int _kill( int pid, int sig );
void _exit( int status );
int _read( int fd, char *buf, int len );
int _write( int fd, const char *buf, int len );
int _close( int fd );
int _fstat( int fd, struct stat *st );
int _isatty( int fd );
int _lseek( int fd, int ptr, int whence );
int _open( char *path, int flags, ... );
int _wait( int *status );
int _unlink( char *name );
int _times( struct tms *buf );
int _stat( const char *file, struct stat *st );
int _link( const char *oldname, const char *newname );
int _fork(void);
int _execve( char *name, char **argv, char **env );

#ifdef USE_FREERTOS

  // TODO: common prty, hook func
  #define FreeRTOS_to_stm32cube_tick_hook \
    void vApplicationTickHook() { HAL_IncTick();  }
  #include <FreeRTOS.h>
  #include <task.h>
  #define OXC_DEFAULT_UART_PRTY configKERNEL_INTERRUPT_PRIORITY
#else
  #define OXC_DEFAULT_UART_PRTY 5
#endif


#ifdef __cplusplus
}
#endif

#ifdef USE_OXC
#define Mu_t  mu_t
#define Mu_lock(x)       mu_lock(x)
#define Mu_unlock(x)     mu_unlock(x)
#define Mu_trylock(x)    mu_trylock(x)
#define Mu_waitlock(x,m) mu_waitlock(x,m)
#define Mu_init          0
#else
#include <pthread.h>
#define Mu_t  pthread_mutex_t
#define Mu_lock(x)       pthread_mutex_lock(x)
#define Mu_unlock(x)     pthread_mutex_unlock(x)
#define Mu_trylock(x)    pthread_mutex_trylock(x)
#define Mu_waitlock(x,m) pthread_mutex_waitlock(x,m)
#define Mu_init          PTHREAD_MUTEX_INITIALIZER
int pthread_mutex_waitlock( pthread_mutex_t *mutex, unsigned ms );
int pthread_mutex_waitlock( pthread_mutex_t *mutex, unsigned ms  )
{
  for( unsigned i=0; i<ms; ++i ) {
    if( pthread_mutex_trylock( mutex ) ) {
        return 1;
    }
  }
  return 0;
}

#endif

#ifdef __cplusplus

class MuLock {
  public:
   MuLock( Mu_t &a_mu ) : mu( a_mu ) { Mu_lock( &mu ); };
   ~MuLock()  { Mu_unlock( &mu ); };
  protected:
   Mu_t &mu;
};

class MuTryLock {
  public:
   MuTryLock( Mu_t &a_mu ) : mu( a_mu ), acq( !Mu_trylock( &mu ) ) { };
   ~MuTryLock()  { if( acq ) { Mu_unlock( &mu ); } };
   bool wasAcq() const { return acq; }
  protected:
   Mu_t &mu;
   const bool acq;
};

class MuWaitLock {
  public:
   MuWaitLock( Mu_t &a_mu, uint32_t ms = 100 ) : mu( a_mu ), acq( !Mu_waitlock( &mu, ms ) ) { };
   ~MuWaitLock()  { if( acq ) { Mu_unlock( &mu ); } };
   bool wasAcq() const { return acq; }
  protected:
   Mu_t &mu;
   const bool acq;
};

#endif

#define USE_DIE_ERROR_HANDLER void Error_Handler( int rc ) { die( rc ); };
#define USE_DIE4LED_ERROR_HANDLER void Error_Handler( int rc ) { die4led( rc ); };
#define USE_DIE_EXIT void void( int rc ) { die( rc ); };
#define USE_DIE4LED_EXIT void void( int rc ) { die4led( rc ); };

#ifndef PROLOG_LED_TIME
#define PROLOG_LED_TIME 200
#endif

#define STD_PROLOG_START \
  HAL_Init(); \
  leds.initHW(); \
  leds.write( BOARD_LEDS_ALL ); \
  int rc = SystemClockCfg(); \
  if( rc ) { \
    die4led( BOARD_LEDS_ALL ); \
    return 1; \
  }

#define STD_PROLOG_UART_NOCON \
  STD_PROLOG_START; \
  delay_ms( PROLOG_LED_TIME ); leds.write( 0x00 ); delay_ms( PROLOG_LED_TIME ); \
  if( ! init_uart( &uah ) ) { \
    die4led( 1 ); \
  } \
  leds.write( BOARD_LEDS_ALL );  HAL_Delay( PROLOG_LED_TIME );

#define STD_PROLOG_UART \
  STD_PROLOG_UART_NOCON; \
  global_smallrl = &srl; \
  SET_UART_AS_STDIO( usartio );

#define STD_PROLOG_USBCDC \
  STD_PROLOG_START; \
  delay_ms( PROLOG_LED_TIME ); leds.write( 0x00 ); delay_ms( PROLOG_LED_TIME ); \
  if( ! usbcdc.init() ) { \
    die4led( 1 ); \
  } \
  leds.write( BOARD_LEDS_ALL );  HAL_Delay( PROLOG_LED_TIME ); \
  global_smallrl = &srl; \
  SET_USBCDC_AS_STDIO( usbcdc );



#define SCHEDULER_START \
  leds.write( 0x00 ); \
  ready_to_start_scheduler = 1; \
  vTaskStartScheduler(); \
  die4led( 0xFF );

#define CREATE_STD_TASKS( SEND_TASK ) \
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr ); \
  xTaskCreate( SEND_TASK,        "send", 2*def_stksz, nullptr,   2, nullptr ); \
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr ); \
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );
  //           code               name    stack_sz      param  prty TaskHandle_t*


#endif

// vim: path=.,/usr/share/stm32cube/inc
