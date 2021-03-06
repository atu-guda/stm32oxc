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

void delay_ms( uint32_t ms )
{
  #ifdef USE_FREERTOS
  if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING ) {
    vTaskDelay( ms / ( ( TickType_t ) 1000 / configTICK_RATE_HZ ) );
  } else {
    delay_bad_ms( ms );
  }
  #else
  HAL_Delay( ms );
  // delay_bad_ms( ms ); // TODO: config
  #endif
}

int delay_ms_brk( uint32_t ms )
{
  #ifdef USE_FREERTOS

  while( ms > 0 ) {
    if( break_flag ) {
      return 1;
    }
    uint32_t cms = ( ms > 10 ) ? 10 : ms;
    if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING ) {
      vTaskDelay( cms / ( ( TickType_t ) 1000 / configTICK_RATE_HZ ) );
    } else {
      delay_bad_ms( cms );
    }
    ms -= cms;
  }

  #else

  uint32_t t0 = HAL_GetTick();
  uint32_t w = ms;

  while( ( HAL_GetTick() - t0 ) < w ) {
    if( break_flag ) {
      return 1;
    }
    // TODO: idle
  }

  #endif
  return 0;
}

int delay_ms_until_brk( uint32_t *tc0, uint32_t ms )
{
  #ifdef USE_FREERTOS

  while( ms > 0 ) {
    if( break_flag ) {
      return 1;
    }
    uint32_t cms = ( ms > 10 ) ? 10 : ms;
    if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING ) {
      vTaskDelayUntil( tc0, cms );
    } else {
      delay_bad_ms( cms );
    }
    ms -= cms;
  }
  return 0;

  #else

  uint32_t t0 = *tc0;
  uint32_t w = ms;
  *tc0 += w;

  while( ( HAL_GetTick() - t0 ) < w ) {
    if( break_flag ) {
      return 1;
    }
    // TODO: idle
  }
  return 0;
  #endif
}



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



void OxcTicker::start()
{
  next = HAL_GetTick() + *pw * q;
}

bool OxcTicker::isTick()
{
  int c = HAL_GetTick();
  if( c < next ) {
    return false;
  }
  next += *pw * q;
  return true;
}


