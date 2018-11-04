#ifndef _OXC_POST_BOARD_CFG_H
#define _OXC_POST_BOARD_CFG_H

#ifndef _BOARD_CFG_DEFINED
#error "_BOARD_CFG_DEFINED not defined"
#endif

#define PREF1 __HAL_RCC_GPIO
#define SUFF1 _CLK_ENABLE()

#define PASTER2(x,y) x ## y
#define EVAL2(x,y)  PASTER2(x,y)
#define PASTER3(a,x,y) a##x##y
#define EVAL3a(a,x,y)  PASTER3(a,x,y)
#define EVAL3(a,x,y)  EVAL3a(a,x,y)
#define GPIO_ENABLER(p) EVAL3(__HAL_RCC_GPIO,p,_CLK_ENABLE())



// ----------------------------- leds ---------------------------
#ifdef BOARD_N_LEDS
#define BOARD_LEDS_ALL  ( (1 << BOARD_N_LEDS) - 1 )
#define BOARD_LEDS_MASK ( BOARD_LEDS_ALL << BOARD_LEDS_OFS )
#define BOARD_LEDS_GPIO_ON GPIO_ENABLER(BOARD_LEDS_GPIOX)
#define BOARD_LEDS_GPIO  EVAL2(GPIO,BOARD_LEDS_GPIOX)

#define BOARD_DEFINE_LEDS PinsOut leds( BOARD_LEDS_GPIO, BOARD_LEDS_OFS, BOARD_N_LEDS );
#endif // BOARD_N_LEDS


// ----------------------------- buttons ---------------------------
#define MK_EXTI_IRQ(q)  EVAL2(q,_IRQn)
#define MK_EXTI_IRQH(q) EVAL2(q,_IRQHandler)

#ifdef BOARD_BTN0_EXIST
#define BOARD_BTN0_BIT     ( 1 << BOARD_BTN0_N )
#define BOARD_BTN0_GPIO_EN GPIO_ENABLER(BOARD_BTN0_GPIOX)
#define BOARD_BTN0_GPIO  EVAL2(GPIO,BOARD_BTN0_GPIOX)
#if BOARD_BTN0_ACTIVE_DOWN == 1
  #define BOARD_BTN0_PULL    GPIO_PULLUP
  #define BOARD_BTN0_MODE    GPIO_MODE_IT_FALLING
#else
  #define BOARD_BTN0_PULL    GPIO_PULLDOWN
  #define BOARD_BTN0_MODE    GPIO_MODE_IT_RISING
#endif

#define BOARD_BTN0_IRQ        MK_EXTI_IRQ(BOARD_BTN0_IRQNAME)
#define BOARD_BTN0_IRQHANDLER MK_EXTI_IRQH(BOARD_BTN0_IRQNAME)
#ifndef BOARD_BTN0_IRQPRTY
  #define BOARD_BTN0_IRQPRTY 14
#endif

#endif // BOARD_BTN0_EXIST

#ifdef BOARD_BTN1_EXIST
#define BOARD_BTN1_BIT     ( 1 << BOARD_BTN1_N )
#define BOARD_BTN1_GPIO_EN GPIO_ENABLER(BOARD_BTN1_GPIOX)
#define BOARD_BTN1_GPIO  EVAL2(GPIO,BOARD_BTN1_GPIOX)
#if BOARD_BTN1_ACTIVE_DOWN == 1
  #define BOARD_BTN1_PULL    GPIO_PULLUP
  #define BOARD_BTN1_MODE    GPIO_MODE_IT_FALLING
#else
  #define BOARD_BTN1_PULL    GPIO_PULLDOWN
  #define BOARD_BTN1_MODE    GPIO_MODE_IT_RISING
#endif

#define BOARD_BTN1_IRQ        MK_EXTI_IRQ(BOARD_BTN1_IRQNAME)
#define BOARD_BTN1_IRQHANDLER MK_EXTI_IRQH(BOARD_BTN1_IRQNAME)
#ifndef BOARD_BTN1_IRQPRTY
  #define BOARD_BTN1_IRQPRTY 14
#endif

#else
#define BOARD_BTN1_BIT     0
#endif // BOARD_BTN1_EXIST


#ifdef __cplusplus

inline constexpr const char* oxc_uart_name( const void *u )
{
  if( u == USART1 ) { return "USART1"; }
  if( u == USART2 ) { return "USART2"; }
  #ifdef USART3
  if( u == USART3 ) { return "USART3"; }
  #endif
  #ifdef USART4
  if( u == USART4 ) { return "USART4"; }
  #endif
  #ifdef USART5
  if( u == USART5 ) { return "USART5"; }
  #endif
  #ifdef USART6
  if( u == USART6 ) { return "USART6"; }
  #endif
  #ifdef USART7
  if( u == USART7 ) { return "USART7"; }
  #endif
  #ifdef USART8
  if( u == USART8 ) { return "USART8"; }
  #endif
  #ifdef UART4
  if( u == UART4 ) { return "UART4"; }
  #endif
  #ifdef UART5
  if( u == UART5 ) { return "UART5"; }
  #endif
  #ifdef UART6
  if( u == UART6 ) { return "UART6"; }
  #endif
  #ifdef UART7
  if( u == UART7 ) { return "UART7"; }
  #endif
  #ifdef UART8
  if( u == UART8 ) { return "UART8"; }
  #endif
  #ifdef UART9
  if( u == UART9 ) { return "UART9"; }
  #endif
  #ifdef UART10
  if( u == UART10 ) { return "UART10"; }
  #endif
  return "?u?";
}
#endif


#endif

