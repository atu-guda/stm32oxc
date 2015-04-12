#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdlib.h>
#endif

#include <oxc_base.h>

#ifdef STD_SYSTICK_HANDLER
#ifdef USE_FREERTOS
#warning Non-RTOS SysTick_Handler defined
#endif
void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

#endif

int exit_rc = 0;

uint8_t numFirstBit( uint32_t a )
{
  for( uint8_t n = 0; n<sizeof(a)*8; ++n ) {
    if( a & 1 ) {
      return n;
    }
    a >>= 1;
  }
  return 0xFF;
}


void taskYieldFun()
{
  #ifdef USE_FREERTOS
    if( xTaskGetSchedulerState() == taskSCHEDULER_RUNNING ) {
      taskYIELD();
    } else {
      delay_bad_mcs( 100 );
    }
  #else
    delay_mcs( 100 );
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



const char hex_digits[] = "0123456789ABCDEFG";
const char dec_digits[] = "0123456789???";



char* char2hex( char c, char *s )
{
  if( s != 0 ) {
    s[0] = hex_digits[ (uint8_t)(c) >> 4 ];
    s[1] = hex_digits[ c & 0x0F ];
    s[2] = 0;
  }
  return s;
}

char* word2hex( uint32_t d,  char *s )
{
  if( s != 0 ) {
    int i;
    for( i=7; i>=0; --i ) {
      s[i] = hex_digits[ d & 0x0F ];
      d >>= 4;
    }
    s[8] = 0;
  }
  return s;
}

char* i2dec( int n, char *s )
{
  static char sbuf[INT_STR_SZ_DEC];
  char tbuf[24];
  unsigned u;
  if( !s ) {
    s = sbuf;
  }
  char *bufptr = s, *tmpptr = tbuf + 1;
  *tbuf = '\0';

  if( n < 0 ){ //  sign
    u = ( (unsigned)(-(1+n)) ) + 1; // INT_MIN handling
    *bufptr++ = '-';
  } else {
    u=n;
  }

  do {
    *tmpptr++ = dec_digits[ u % 10 ];
  } while( u /= 10 );

  while( ( *bufptr++ = *--tmpptr ) != '\0' ) /* NOP */;
  return s;
}

