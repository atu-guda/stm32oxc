#ifndef _OXC_BASE_H
#define _OXC_BASE_H

#include <oxc_archdef.h>


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

#include "board_cfg.h"

#include <oxc_post_board_cfg.h>


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

// extern  const int _RAM_SIZE, _FLASH_SIZE, _CCMRAM_SIZE; // not work? 0xFFFFFFFF
extern  const int _sdata, _edata, _sbss, _ebss, _end, _estack;

extern char* __heap_top;
extern int ready_to_start_scheduler;
extern int exit_rc;
extern volatile int dbg_val0, dbg_val1, dbg_val2, dbg_val3;
extern volatile int idle_flag;
extern volatile int break_flag;
extern volatile int sigint_count;
#ifndef TASK_LEDS_QUANT
  #define TASK_LEDS_QUANT 10
#endif
// delay is TASK_LEDS_QUANT * task_leds_step,
extern volatile int task_leds_step; // initial = 50

typedef __IO uint32_t reg32;
typedef const char *const ccstr;
#define BAD_ADDR ((void*)(0xFFFFFFFF))

#define ARR_SZ(x) (sizeof(x) / sizeof(x[0]))
#define ARR_AND_SZ(x) x, (sizeof(x) / sizeof(x[0]))

inline constexpr uint32_t make_bit_mask( uint8_t start, uint8_t n )
{
  return ( (uint32_t)(0xFFFFFFFF) << ( 32 - n ) ) >> ( 32 - n - start );
}
inline constexpr uint32_t make_bit_mask_left( uint8_t n )
{
  return ( (uint32_t)(0xFFFFFFFF) >> ( 32 - n ) );
}
inline constexpr uint16_t make_gpio_mask( uint8_t start, uint8_t n )
{
  return (uint16_t) make_bit_mask( start, n );
}

inline bool check_bit( uint32_t v, uint8_t pos )
{
  return v & ( 1u << pos );
}

#ifdef __cplusplus
#include <type_traits>

template<typename T>
inline void set_bit( T &v, uint8_t pos )
{
  v |= 1 << pos;
}

template<typename T>
inline void set_bits( T &v, uint8_t pos, uint8_t n )
{
  v |= make_bit_mask( pos, n );
}

template<typename T>
inline void reset_bit( T &v, uint8_t pos )
{
  v &= ~( 1u << pos );
}

template<typename T>
inline void reset_bits( T &v, uint8_t pos, uint8_t n )
{
  v &= ~ make_bit_mask( pos, n );
}

template<typename T>
inline void replace_bits( T &v, uint8_t pos, uint8_t n, uint32_t bits )
{
  typename std::remove_volatile<T>::type t = v;
  t &= ~ make_bit_mask( pos, n );
  t |= ( bits << pos );
  v = t;
}
#endif


#ifndef NL
  #define NL "\n"
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


// void die4led( uint16_t n );
void taskYieldFun(void);
void wakeFromIRQ( long wake );
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
void delay_bad_100ns( uint32_t ns100 );

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

typedef void (*AuxTickFun)(void);
#define AUX_TICK_FUN_N 4
extern AuxTickFun oxc_aux_tick_funcs[AUX_TICK_FUN_N];
int  oxc_add_aux_tick_fun( AuxTickFun f );
int  oxc_del_aux_tick_fun( AuxTickFun f );
void oxc_clear_aux_tick_funs(void);
void oxc_call_aux_tick_funcs(void);

#ifdef USE_FREERTOS

  #include <FreeRTOS.h>
  #include <task.h>
  #define OXC_DEFAULT_UART_PRTY configKERNEL_INTERRUPT_PRIORITY
#else
  #define OXC_DEFAULT_UART_PRTY 5
#endif


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

class Chst { // char + status
  public:
   enum {
     st_good = 0, st_full = 1, st_empty = 2, st_lock = 4
   };
   Chst( char ch ) : c( ch ), st( st_good ) {};
   Chst( char ch, uint8_t a_st ) : c( ch ), st( a_st ) {};
   bool good()   const noexcept { return st == st_good;  }
   bool full()   const noexcept { return st == st_full;  }
   bool empty()  const noexcept { return st == st_empty; }
   bool locked() const noexcept { return st == st_lock;  }

   char c;
   uint8_t st;
};
static_assert( sizeof(Chst) == 2, "Bad size of Chst struct, !=2" );

class OxcTicker {
  public:
    explicit OxcTicker( int aw )   : w( aw ),    pw( &w )           { start(); } ;
    OxcTicker( volatile int *apw, int qsz ) : w( *apw ), pw( apw ), q( qsz ) { start(); } ;
    bool isTick();
    void start();
    void setW( int aw ) { w = aw; }
  protected:
    int w, next;
    volatile int *pw;
    int q = 1;
};

#endif


void default_wait1(void);

#define USE_DIE_ERROR_HANDLER void Error_Handler( int rc ) { die( rc ); };
#define USE_DIE4LED_ERROR_HANDLER void Error_Handler( int rc ) { die4led( rc ); };
#define USE_DIE_EXIT void void( int rc ) { die( rc ); };
#define USE_DIE4LED_EXIT void void( int rc ) { die4led( rc ); };

#ifndef PROLOG_LED_TIME
#define PROLOG_LED_TIME  50
#endif

#ifdef USE_OXC_DEVIO
#define ADD_DEVIO_TICKFUN oxc_add_aux_tick_fun( DevIO::tick_actions_all )
#else
#define ADD_DEVIO_TICKFUN
#endif

#define STD_PROLOG_START \
  HAL_Init(); \
  leds.initHW(); \
  leds.write( BOARD_LEDS_ALL ); \
  int rc = SystemClockCfg(); \
  if( rc ) { \
    die4led( BOARD_LEDS_ALL ); \
    return 1; \
  } \
  ADD_DEVIO_TICKFUN;

#define STD_PROLOG_UART_NOCON \
  STD_PROLOG_START; \
  delay_ms( PROLOG_LED_TIME ); leds.write( 0x00 ); delay_ms( PROLOG_LED_TIME ); \
  if( ! init_uart( &uah_console ) ) { \
    die4led( 1 ); \
  } \
  leds.write( BOARD_LEDS_ALL );  HAL_Delay( PROLOG_LED_TIME );

#define STD_PROLOG_UART \
  STD_PROLOG_UART_NOCON; \
  global_smallrl = &srl; \
  SET_UART_AS_STDIO( dev_console ); \
  std_out.setOut( devio_fds[1] );

#define STD_PROLOG_USBCDC \
  STD_PROLOG_START; \
  delay_ms( PROLOG_LED_TIME ); leds.write( 0x00 ); delay_ms( PROLOG_LED_TIME ); \
  if( ! dev_console.init() ) { \
    die4led( 1 ); \
  } \
  leds.write( BOARD_LEDS_ALL );  HAL_Delay( PROLOG_LED_TIME ); \
  global_smallrl = &srl; \
  SET_USBCDC_AS_STDIO( dev_console ); \
  std_out.setOut( devio_fds[1] );



#define SCHEDULER_START \
  leds.write( 0x00 ); \
  ready_to_start_scheduler = 1; \
  vTaskStartScheduler(); \
  die4led( 0xFF );

#define CREATE_STD_TASKS \
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr ); \
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr ); \
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );
  //           code               name    stack_sz      param  prty TaskHandle_t*


#endif

// vim: path=.,/usr/share/stm32cube/inc
