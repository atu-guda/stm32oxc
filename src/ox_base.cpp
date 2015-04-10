#ifdef USE_FREERTOS
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdlib.h>
#endif

#include <ox_base.h>

#if defined(STM32F1)

GPIO_InitTypeDef GPIO_Modes[pinMode_MAX] = {
  { 0, GPIO_DFL_Speed, GPIO_Mode_IN_FLOATING  },// None - fake INF
  { 0, GPIO_DFL_Speed, GPIO_Mode_AIN },         // AN
  { 0, GPIO_DFL_Speed, GPIO_Mode_IN_FLOATING }, // INF
  { 0, GPIO_DFL_Speed, GPIO_Mode_IPU },         // IPU
  { 0, GPIO_DFL_Speed, GPIO_Mode_IPD },         // IPD
  { 0, GPIO_DFL_Speed, GPIO_Mode_Out_PP },      // Out_PP
  { 0, GPIO_DFL_Speed, GPIO_Mode_Out_OD },      // Out_OD
  { 0, GPIO_DFL_Speed, GPIO_Mode_AF_PP },       // AF_PP
  { 0, GPIO_DFL_Speed, GPIO_Mode_AF_OD },       // AF_OD
  { 0, GPIO_DFL_Speed, GPIO_Mode_IPU }          // AFIU = IPU for f1
};
#else // 2.3.4 is similar?

GPIO_InitTypeDef GPIO_Modes[pinMode_MAX] = {
  { 0, GPIO_Mode_IN,  GPIO_DFL_Speed, GPIO_OType_PP, GPIO_PuPd_NOPULL  }, // None - fake INF
  { 0, GPIO_Mode_AN,  GPIO_DFL_Speed, GPIO_OType_PP, GPIO_PuPd_NOPULL  }, // AN
  { 0, GPIO_Mode_IN,  GPIO_DFL_Speed, GPIO_OType_PP, GPIO_PuPd_NOPULL  }, // INF
  { 0, GPIO_Mode_IN,  GPIO_DFL_Speed, GPIO_OType_PP, GPIO_PuPd_UP      }, // IPU
  { 0, GPIO_Mode_IN,  GPIO_DFL_Speed, GPIO_OType_PP, GPIO_PuPd_DOWN    }, // IPD
  { 0, GPIO_Mode_OUT, GPIO_DFL_Speed, GPIO_OType_PP, GPIO_PuPd_NOPULL  }, // Out_PP
  { 0, GPIO_Mode_OUT, GPIO_DFL_Speed, GPIO_OType_OD, GPIO_PuPd_NOPULL  }, // Out_OD
  { 0, GPIO_Mode_AF,  GPIO_DFL_Speed, GPIO_OType_PP, GPIO_PuPd_NOPULL  }, // AF_PP
  { 0, GPIO_Mode_AF,  GPIO_DFL_Speed, GPIO_OType_OD, GPIO_PuPd_NOPULL  }, // AF_OD
  { 0, GPIO_Mode_AF,  GPIO_DFL_Speed, GPIO_OType_PP, GPIO_PuPd_UP      }  // AFIU
};

#endif

void devPinsConf( GPIO_TypeDef* GPIOx, enum PinModeNum mode_num, uint16_t pins )
{
  GPIO_InitTypeDef gp = GPIO_Modes[mode_num];
  gp.GPIO_Pin = pins;
  GPIO_Init( GPIOx, &gp );
}

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

void GPIO_WriteBits( GPIO_TypeDef* GPIOx, uint16_t PortVal, uint16_t mask )
{
  GPIOx->ODR = ( PortVal & mask ) | ( GPIOx->ODR & (~mask) );
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
  delay_bad_ms( ms );
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

