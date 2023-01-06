#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdlib.h>
#endif

#include <oxc_base.h>

char* __heap_top = (char*)(&_end);

int ready_to_start_scheduler = 0;
int exit_rc = 0;
volatile int dbg_val0 = 0, dbg_val1 = 0, dbg_val2 = 0, dbg_val3 = 0;
volatile int idle_flag = 0;
volatile int break_flag = 0;
volatile int sigint_count   =  0;
volatile int task_leds_step = 50; // 500 ms def


void default_wait1()
{
  delay_ms( 1 );
}

AuxTickFun oxc_aux_tick_funcs[AUX_TICK_FUN_N];

int  oxc_add_aux_tick_fun( AuxTickFun f )
{
  int rc = -1;
  oxc_disable_interrupts(); // TODO: store/restore
  for( int i=0; i< AUX_TICK_FUN_N; ++i ) {
    if( oxc_aux_tick_funcs[i]  == nullptr ) {
      oxc_aux_tick_funcs[i] = f;
      rc = i;
      break;
    }
  }
  oxc_enable_interrupts();
  return rc;
}

int  oxc_del_aux_tick_fun( AuxTickFun f )
{
  int rc = -1;
  oxc_disable_interrupts(); // TODO: store/restore
  for( int i=0; i< AUX_TICK_FUN_N; ++i ) {
    if( oxc_aux_tick_funcs[i]  == f ) {
      oxc_aux_tick_funcs[i] = nullptr;
      rc = i;
      break;
    }
  }
  oxc_enable_interrupts();
  return rc;
}

void oxc_clear_aux_tick_funs()
{
  oxc_disable_interrupts(); // TODO: store/restore
  for( int i=0; i< AUX_TICK_FUN_N; ++i ) {
    oxc_aux_tick_funcs[i] = nullptr;
  }
  oxc_enable_interrupts();
}

void oxc_call_aux_tick_funcs()
{
  for( int i=0; i< AUX_TICK_FUN_N; ++i ) {
    if( oxc_aux_tick_funcs[i]  != nullptr ) {
      oxc_aux_tick_funcs[i]();
    }
  }
}

#ifndef NO_STD_SYSTICK_HANDLER
#ifdef USE_FREERTOS
extern "C" {
void xPortSysTickHandler();
};
void SysTick_Handler(void)
{
  HAL_IncTick();
  if( ready_to_start_scheduler ) {
    xPortSysTickHandler();
  } else {
    HAL_SYSTICK_IRQHandler();
  }
  oxc_call_aux_tick_funcs();
}
#else
void SysTick_Handler(void)
{
  HAL_IncTick();
  oxc_call_aux_tick_funcs();
  HAL_SYSTICK_IRQHandler();
}
#endif
#endif


void taskYieldFun()
{
  #ifdef USE_FREERTOS
    if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING ) {
      taskYIELD();
    } else {
      delay_bad_mcs( 1 );
    }
  #else
    delay_mcs( 1 );
  #endif
}

void wakeFromIRQ( long wake )
{
  #ifdef USE_FREERTOS
  portEND_SWITCHING_ISR( wake );
  #else
  UNUSED( wake );
  #endif
}

[[ noreturn ]] void die( uint16_t n )
{
  #ifdef USE_FREERTOS
  taskDISABLE_INTERRUPTS();
  #else
  oxc_disable_interrupts();
  #endif
  while(1) { delay_bad_ms( n*200 ); /* NOP */ };
}

uint32_t delay_calibrate_value = 200 * 16; // for initial 16 MHz

// fallback values
#ifndef DELAY_APPROX_COEFF
  #if   defined (STM32F0)
    #define DELAY_APPROX_COEFF 9060
  #elif defined (STM32F1)
    #define DELAY_APPROX_COEFF 7040
  #elif defined (STM32F3)
    #define DELAY_APPROX_COEFF 7040
  #elif defined (STM32F4)
    #define DELAY_APPROX_COEFF 5010
  #elif defined (STM32F7)
    #define DELAY_APPROX_COEFF 2000
  #endif
#endif

void approx_delay_calibrate()
{
  delay_calibrate_value = HAL_RCC_GetSysClockFreq() / DELAY_APPROX_COEFF;
}

void do_delay_calibrate()
{
  uint32_t n = delay_calibrate_value * 500; // for calibrate 500 ms
  uint32_t tm0 = HAL_GetTick();
  delay_bad_n( n );
  uint32_t tm1 = HAL_GetTick(), dlt = tm1 - tm0;
  delay_calibrate_value = n / dlt;
}

void delay_bad_mcs( uint32_t mcs )
{
  uint32_t n = mcs * delay_calibrate_value / 1000;
  for( uint32_t i=0; i<n; ++i ) {
    __asm volatile ( "nop;");
  }
}

void delay_bad_100ns( uint32_t ns100 )
{
  uint32_t n = ns100 * delay_calibrate_value / 10000;
  for( uint32_t i=0; i<n; ++i ) {
    __asm volatile ( "nop;");
  }
}

#ifndef OXC_USE_STD_HAL_DELAY
void HAL_Delay( uint32_t Delay )
{
  return  (void)delay_ms_until_brk_ex( nullptr, Delay, false );
}
#endif

void delay_ms( uint32_t ms )
{
  return (void)delay_ms_until_brk_ex( nullptr, ms, false );
}

int delay_ms_brk( uint32_t ms )
{
  return delay_ms_until_brk_ex( nullptr, ms, true );
}

int delay_ms_until_brk( uint32_t *tc0, uint32_t ms )
{
  return delay_ms_until_brk_ex( tc0, ms, true );
}

#ifdef USE_FREERTOS

int  delay_ms_until_brk_ex( uint32_t *tc0, uint32_t ms, bool check_break ) // FreeRTOS version
{
  if( __get_PRIMASK() ) {
    delay_bad_ms( ms );
    return 0;
  }

  TickType tc0_local;
  if( ! tc0 ) {
    tc0_local = GET_OS_TICK();
    tc0 = &tc0_local;
  }

  while( ms > 0 ) {
    if( check_break && break_flag ) {
      return 1;
    }
    uint32_t cms = ( ms > OXC_DELAY_DEFAULT_CHECK ) ? OXC_DELAY_DEFAULT_CHECK : ms;
    if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING ) {
      vTaskDelayUntil( tc0, cms );
    } else {
      delay_bad_ms( cms );
    }
    ms -= cms;
  }
  return 0;
}

// not FreeRTOS
#else


int  delay_ms_until_brk_ex( uint32_t *tc0, uint32_t ms, bool check_break )
{
  if( __get_PRIMASK() ) {
    delay_bad_ms( ms );
    return 0;
  }

  uint32_t t0 = tc0 ? *tc0 : HAL_GetTick();
  if( ms < 1 ) {
    // TODO: catch 0-call
    ++ms;
  }

  if( tc0 ) {
    *tc0 += ms;
  }

  while( ( HAL_GetTick() - t0 ) < ms ) {
    if( check_break && break_flag ) {
      return 1;
    }
    // TODO: idle
  }
  return 0;
}

#endif
// end FreeRTOS/not



void delay_bad_n( uint32_t dly )
{
  for( uint32_t i=0; i<dly; ++i ) {
    __asm volatile ( "nop;");
  }
}

void delay_bad_s( uint32_t s )
{
  uint32_t n = s * delay_calibrate_value * 1000;
  for( uint32_t i=0; i<n; ++i ) {
    __asm volatile ( "nop;");
  }
}

void delay_bad_ms( uint32_t ms )
{
  uint32_t n = ms * delay_calibrate_value;
  for(  uint32_t i = 0; i<n; ++i ) {
    __asm volatile ( "nop;");
  }
}

void delay_mcs( uint32_t mcs )
{
  int ms = mcs / 1000;
  int mcs_r = mcs % 1000;
  if( ms ) {
    delay_ms( ms );
  }
  if( mcs_r ) {
    delay_bad_mcs( mcs_r );
  }
}





