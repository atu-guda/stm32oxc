#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdlib.h>
#endif

#include <oxc_base.h>

int exit_rc = 0;
volatile int dbg_val0 = 0, dbg_val1 = 0, dbg_val2 = 0, dbg_val3 = 0;

#if STD_SYSTICK_HANDLER != 0
#ifdef USE_FREERTOS
#warning Non-RTOS SysTick_Handler defined
#endif
void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

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


void delay_bad_mcs( uint32_t mcs )
{
  uint32_t n = mcs * T_MKS_MUL;
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


void delay_bad_n( uint32_t dly )
{
  for( uint32_t i=0; i<dly; ++i ) {
    __asm volatile ( "nop;");
  }
}

void delay_bad_s( uint32_t s )
{
  uint32_t n = s * T_S_MUL;
  for( uint32_t i=0; i<n; ++i ) {
    __asm volatile ( "nop;");
  }
}

void delay_bad_ms( uint32_t ms )
{
  uint32_t n = ms * T_MS_MUL;
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




