#ifndef _OXC_BASE_H
#define _OXC_BASE_H

#ifdef __cplusplus
  #include <type_traits>
  #include <utility>
#endif

#include <oxc_archdef.h>

#if REQ_MCBASE != MCBASE
  #error "Required and given MCBASE is not equal"
#endif

#include <oxc_types.h>
#include <oxc_irqlist.h>

#include "board_cfg.h"

#include <oxc_post_board_cfg.h>


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
 extern "C" {
#endif

inline void oxc_enable_interrupts(void)
{
  __asm__ volatile ( "CPSIE I" : : : "memory");
}

inline void oxc_disable_interrupts(void)
{
  __asm__ volatile ( "CPSID I" : : : "memory" );
}


inline void oxc_dmb(void)
{
  __asm__ volatile ( "DMB 0xF" : : : "memory" );
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


#ifndef AUX_TICK_FUN_N
  #define AUX_TICK_FUN_N 4
#endif

#ifdef __cplusplus
}
#endif

extern AuxTickFun oxc_aux_tick_funcs[AUX_TICK_FUN_N];
ReturnCode  oxc_add_aux_tick_fun( AuxTickFun f );
ReturnCode  oxc_del_aux_tick_fun( AuxTickFun f );
void oxc_clear_aux_tick_funs(void);
void oxc_call_aux_tick_funcs(void);


#define USE_DIE_ERROR_HANDLER void Error_Handler( int rc ) { die( rc ); };
#define USE_DIE4LED_ERROR_HANDLER void Error_Handler( PinMask rc ) { die4led( rc ); };
#define USE_DIE_EXIT void exit( int rc ) { die( rc ); };
#define USE_DIE4LED_EXIT void exit( int rc ) { die4led( rc ); };


#ifdef USE_FREERTOS
  #include <FreeRTOS.h>
  #include <task.h>
  typedef TickType_t TickType;
  #define GET_OS_TICK xTaskGetTickCount
  #ifndef OXC_DELAY_DEFAULT_CHECK
    #define OXC_DELAY_DEFAULT_CHECK 10
  #endif
  #ifndef OXC_DEFAULT_UART_PRTY
    #define OXC_DEFAULT_UART_PRTY configKERNEL_INTERRUPT_PRIORITY
  #endif
#else
  typedef uint32_t TickType;
  #define GET_OS_TICK HAL_GetTick
  #ifndef OXC_DEFAULT_UART_PRTY
    #define OXC_DEFAULT_UART_PRTY 5
  #endif
#endif


#ifdef __cplusplus
template <typename F, typename... Args > auto at_disabled_irq( F fun, Args&&... args )
{
  bool was_irqs_enabled = ( __get_PRIMASK() == 0 );
  oxc_disable_interrupts();
  if constexpr( std::is_void_v<decltype( fun(std::forward<Args>(args)...) )>) {
    fun( std::forward<Args>(args)... );
    if( was_irqs_enabled ) {
      oxc_enable_interrupts();
    }
  } else {
    auto res = fun( std::forward<Args>(args)... );
    if( was_irqs_enabled ) {
      oxc_enable_interrupts();
    }
    return res;
  }
}
#endif

#endif

// vim: path=.,/usr/share/stm32cube/inc
