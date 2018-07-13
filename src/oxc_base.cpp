#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdlib.h>
#endif

#include <oxc_base.h>

extern  const int _sdata, _edata, _sbss, _ebss, _end, _estack;
char* __heap_top = (char*)(&_end);

int ready_to_start_scheduler = 0;
int exit_rc = 0;
volatile int dbg_val0 = 0, dbg_val1 = 0, dbg_val2 = 0, dbg_val3 = 0;

void mu_lock( mu_t *m )
{
  // oxc_disable_interrupts();
  while( !mu_trylock( m ) ) { /* NOP */ ; };
  // oxc_enable_interrupts();
}

uint32_t mu_trylock( mu_t *m ) // returns 1 - lock is acquired
{
  uint32_t sta = 1;

  if( oxc_ldrex( m ) == 0 ) { // unlocked
    sta = oxc_strex( 1, m ); // try to lock
  }
  oxc_dmb();

  return sta == 0;
}

void mu_unlock( mu_t *m )
{
  oxc_dmb();
  *m = 0;
}


#ifndef NO_STD_SYSTICK_HANDLER
#ifdef USE_FREERTOS
extern "C" {
void xPortSysTickHandler();
};
void SysTick_Handler(void)
{
  if( ready_to_start_scheduler ) {
    xPortSysTickHandler();
  } else {
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
  }
}
#else
void SysTick_Handler(void)
{
  HAL_IncTick();
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


void die( uint16_t n )
{
  #ifdef USE_FREERTOS
  taskDISABLE_INTERRUPTS();
  #endif
  while(1) { delay_bad_ms( n*200 ); /* NOP */ };
}

uint32_t delay_calibrate_value = 200 * 16; // for initial 16 MHz

void approx_delay_calibrate()
{
  delay_calibrate_value = 200 * ( HAL_RCC_GetSysClockFreq() / 1000000 );
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

  while( ms > 0 ) {
    if( break_flag ) {
      return 1;
    }
    uint32_t cms = ( ms > 10 ) ? 10 : ms;
    HAL_Delay( cms );
    ms -= cms;
  }
  // delay_bad_ms( ms ); // TODO: config
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
  return delay_ms_brk( ms ); // TODO: real?
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
  if( ms )
    delay_ms( ms );
  if( mcs_r )
    delay_bad_mcs( mcs_r );
}




