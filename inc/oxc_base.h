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

#define BAD_ADDR ((void*)(0xFFFFFFFF))
#ifndef NL
  #define NL "\n"
#endif

typedef __IO uint32_t reg32;
typedef const char *const ccstr;
typedef uint32_t mu_t; // mutex_t alike

// from ../ld/stm32_common_base.ld
extern  const int _sdata, _edata, _sbss, _ebss, _end, _estack;

extern char* __heap_top;
extern int ready_to_start_scheduler;
extern int exit_rc;
extern volatile int dbg_val0, dbg_val1, dbg_val2, dbg_val3;
extern volatile int idle_flag;
extern volatile int break_flag;
extern volatile int sigint_count;
extern uint32_t delay_calibrate_value;
#ifndef TASK_LEDS_QUANT
  #define TASK_LEDS_QUANT 10
#endif
// delay is TASK_LEDS_QUANT * task_leds_step,
extern volatile int task_leds_step; // initial = 50


#ifdef __cplusplus
 template<typename T> class _ShowType; // to output deducted type
 //                        // _ShowType< decltype(XXXX) > xType;

 template<typename T> // TODO: array/container only
  void fill_0( T &a ) { for( auto &x : a ) { x = 0; } }
 template<typename T, typename V >
  void fill_val( T &a, V v ) { for( auto &x : a ) { x = v; } }

 extern "C" {
#endif

inline void oxc_enable_interrupts(void)
{
  __asm__ volatile ( "CPSIE I" );
}

inline void oxc_disable_interrupts(void)
{
  __asm__ volatile ( "CPSID I" );
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
void die( uint16_t n ) __attribute__((noreturn));
void Error_Handler( int rc ); // defined at user program

void approx_delay_calibrate(void);
void do_delay_calibrate(void);
void delay_ms( uint32_t ms ); // base on vTaskDelay - switch to scheduler (if avail), or to HAL
int  delay_ms_brk( uint32_t ms ); // exit with 1 if break_flag is set;
int  delay_ms_until_brk( uint32_t *tc0, uint32_t ms );
int  delay_ms_until_brk_ex( uint32_t *tc0, uint32_t ms, int check_break );
void delay_mcs( uint32_t mcs );
// dumb delay functions - loop based - for use w/o timer and for small times
void delay_bad_n( uint32_t n );
void delay_bad_s( uint32_t s );
void delay_bad_ms( uint32_t ms );
void delay_bad_mcs( uint32_t mcs );
void delay_bad_100ns( uint32_t ns100 );
void default_wait1(void);
int on_delay_actions(void); // called from non-FreeRTOS delay_*,
                        // returns: 0: continue delay, 1+ - break
                        // must be fast

void SystemClock_Config(void);
int  SystemClockCfg(void); // returns: 0: ok >0 + set errno: error


typedef void (*AuxTickFun)(void);
#ifndef AUX_TICK_FUN_N
  #define AUX_TICK_FUN_N 4
#endif
extern AuxTickFun oxc_aux_tick_funcs[AUX_TICK_FUN_N];
int  oxc_add_aux_tick_fun( AuxTickFun f );
int  oxc_del_aux_tick_fun( AuxTickFun f );
void oxc_clear_aux_tick_funs(void);
void oxc_call_aux_tick_funcs(void);

#ifdef __cplusplus
}
#endif



#define USE_DIE_ERROR_HANDLER void Error_Handler( int rc ) { die( rc ); };
#define USE_DIE4LED_ERROR_HANDLER void Error_Handler( int rc ) { die4led( rc ); };
#define USE_DIE_EXIT void exit( int rc ) { die( rc ); };
#define USE_DIE4LED_EXIT void exit( int rc ) { die4led( rc ); };

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

#ifdef USE_FREERTOS

  #include <FreeRTOS.h>
  #include <task.h>
  #define GET_OS_TICK xTaskGetTickCount
  typedef TickType_t TickType;
  #ifndef OXC_DELAY_DEFAULT_CHECK
    #define OXC_DELAY_DEFAULT_CHECK 10
  #endif

  #define OXC_DEFAULT_UART_PRTY configKERNEL_INTERRUPT_PRIORITY
  //
  #define SCHEDULER_START \
    leds.write( 0x00 ); \
    ready_to_start_scheduler = 1; \
    vTaskStartScheduler(); \
    die4led( 0xFF );
  //
  #define CREATE_STD_TASKS \
    xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr ); \
    xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr ); \
    xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );
    //           code               name    stack_sz      param  prty TaskHandle_t*
#else
  #define GET_OS_TICK HAL_GetTick
  typedef uint32_t TickType;
  #ifndef OXC_DEFAULT_UART_PRTY
    #define OXC_DEFAULT_UART_PRTY 5
  #endif
#endif



#endif

// vim: path=.,/usr/share/stm32cube/inc
