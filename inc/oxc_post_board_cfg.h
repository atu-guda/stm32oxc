#ifndef _OXC_POST_BOARD_CFG_H
#define _OXC_POST_BOARD_CFG_H

#ifndef _BOARD_CFG_DEFINED
#error "_BOARD_CFG_DEFINED not defined"
#endif




// ----------------------------- leds ---------------------------
#ifdef BOARD_N_LEDS
  inline constexpr uint16_t BOARD_LEDS_ALL {  (1 << BOARD_N_LEDS) - 1 };
  #define BOARD_DEFINE_LEDS PinsOut leds( BOARD_LEDS_START, BOARD_N_LEDS );
#endif // BOARD_N_LEDS


// ----------------------------- buttons ---------------------------

#ifdef BOARD_BTN0_N
  #define BOARD_BTN0 OXC_EVAL3( P, BOARD_BTN0_GPIOX, BOARD_BTN0_N )
  #if BOARD_BTN0_ACTIVE_DOWN == 1
    #define BOARD_BTN0_PULL    GpioPull::up
    #define BOARD_BTN0_MODE    ExtiEv::down
  #else
    #define BOARD_BTN0_PULL    GpioPull::down
    #define BOARD_BTN0_MODE    ExtiEv::up
  #endif

  #ifndef BOARD_BTN0_IRQPRTY
    #define BOARD_BTN0_IRQPRTY 14
  #endif

#endif // BOARD_BTN0_N

#ifdef BOARD_BTN1_N
  #define BOARD_BTN1 OXC_EVAL3( P, BOARD_BTN1_GPIOX, BOARD_BTN1_N )
  #if BOARD_BTN1_ACTIVE_DOWN == 1
    #define BOARD_BTN1_PULL    GpioPull::up
    #define BOARD_BTN1_MODE    ExtiEv::down
  #else
    #define BOARD_BTN1_PULL    GpioPull::down
    #define BOARD_BTN1_MODE    ExtiEv::up
  #endif

  #ifndef BOARD_BTN1_IRQPRTY
    #define BOARD_BTN1_IRQPRTY 14
  #endif

#endif // BOARD_BTN1_N

// use defines for inc/<arch>/oxc_archdef.h
#define BOARD_BTN0_IRQHANDLER OXC_EVAL2(EXTI_HANDLER_, BOARD_BTN0_N)
#define BOARD_BTN0_IRQ        OXC_EVAL2(EXTI_IRQ_,     BOARD_BTN0_N)
#define BOARD_BTN1_IRQHANDLER OXC_EVAL2(EXTI_HANDLER_, BOARD_BTN1_N)
#define BOARD_BTN1_IRQ        OXC_EVAL2(EXTI_IRQ_,     BOARD_BTN1_N)

#define TO_MACRO_STR(x) #x

// names
#ifdef TIM_EXA
  #define TIM_EXA_NAME    TO_MACRO_STR(TIM_EXA)
#endif

#ifdef BOARD_I2C_DEFAULT
  #define  BOARD_I2C_DEFAULT_NAME  TO_MACRO_STR(BOARD_I2C_DEFAULT)
#endif

#ifdef BOARD_SPI_DEFAULT
  #define BOARD_SPI_DEFAULT_NAME TO_MACRO_STR(BOARD_SPI_DEFAULT)
#endif

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

