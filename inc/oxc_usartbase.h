#ifndef _OXC_USARTBASE_H
#define _OXC_USARTBASE_H

// to prevent warnings
#ifndef USE_HAL_UART_REGISTER_CALLBACKS
  #define USE_HAL_UART_REGISTER_CALLBACKS 0
#endif
#ifndef USE_HAL_USART_REGISTER_CALLBACKS
  #define USE_HAL_USART_REGISTER_CALLBACKS 0
#endif

#if defined (STM32F0)
 #include <stm32f0xx_hal_usart.h>
 #define USART_TX_REG TDR
 #define USART_RX_REG RDR
 #define USART_SR_REG ISR
#elif defined (STM32F1)
 #include <stm32f1xx_hal_usart.h>
 #define USART_TX_REG DR
 #define USART_RX_REG DR
 #define USART_SR_REG SR
#elif defined (STM32F2)
 #include <stm32f2xx_hal_usart.h>
 #define USART_TX_REG DR
 #define USART_RX_REG DR
 #define USART_SR_REG SR
#elif defined (STM32F3)
 #include <stm32f3xx_hal_usart.h>
 #define USART_TX_REG TDR
 #define USART_RX_REG RDR
 #define USART_SR_REG ISR
#elif defined (STM32F4)
 #include <stm32f4xx_hal_uart.h>
 #include <stm32f4xx_hal_usart.h>
 #define USART_TX_REG DR
 #define USART_RX_REG DR
 #define USART_SR_REG SR
#elif defined(STM32F7)
 #include <stm32f7xx_hal_usart.h>
 #define USART_TX_REG TDR
 #define USART_RX_REG RDR
 #define USART_SR_REG ISR
#elif defined(STM32H7)
 #include <stm32h7xx_hal_usart.h>
 #define USART_TX_REG TDR
 #define USART_RX_REG RDR
 #define USART_SR_REG ISR
#elif defined(STM32G4)
 #include <stm32g4xx_hal_usart.h>
 #define USART_TX_REG TDR
 #define USART_RX_REG RDR
 #define USART_SR_REG ISR
#else
  #error "Unsupported MCU"
#endif


// common declarations
int init_uart( UART_HandleTypeDef *uah, int baud = 115200 ); // must be provided by project

extern "C" {
  void USART1_IRQHandler(void);
  void USART2_IRQHandler(void);
  void USART3_IRQHandler(void);
  void USART4_IRQHandler(void);
  void USART5_IRQHandler(void);
  void USART6_IRQHandler(void);
  void UART7_IRQHandler(void);
  void UART8_IRQHandler(void);
  void task_usart1_send( void *prm UNUSED_ARG ); // UNUSED now
  void task_usart1_recv( void *prm UNUSED_ARG );
  void task_usart2_send( void *prm UNUSED_ARG );
  void task_usart2_recv( void *prm UNUSED_ARG );
  void task_usart3_send( void *prm UNUSED_ARG );
  void task_usart3_recv( void *prm UNUSED_ARG );
  void task_usart4_send( void *prm UNUSED_ARG );
  void task_usart4_recv( void *prm UNUSED_ARG );
  void task_usart5_send( void *prm UNUSED_ARG );
  void task_usart5_recv( void *prm UNUSED_ARG );
  void task_usart6_send( void *prm UNUSED_ARG );
  void task_usart6_recv( void *prm UNUSED_ARG );
  void task_uart7_send( void *prm UNUSED_ARG );
  void task_uart7_recv( void *prm UNUSED_ARG );
  void task_uart8_send( void *prm UNUSED_ARG );
  void task_uart8_recv( void *prm UNUSED_ARG );
}

#define STD_USART1_IRQ( obj ) void USART1_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART2_IRQ( obj ) void USART2_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART3_IRQ( obj ) void USART3_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART4_IRQ( obj ) void USART4_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART5_IRQ( obj ) void USART5_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART6_IRQ( obj ) void USART6_IRQHandler(void) { obj.handleIRQ(); }
#define STD_UART7_IRQ( obj )  void UART7_IRQHandler(void)  { obj.handleIRQ(); }
#define STD_UART8_IRQ( obj )  void UART8_IRQHandler(void)  { obj.handleIRQ(); }

#endif
// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include
