#ifndef _OXC_USARTIO_H
#define _OXC_USARTIO_H

#include <oxc_devio.h>


class UsartIO : public DevIO {
  public:
   enum {
     CR1_UE  = 0x2000,  //* Enable
     CR1_SBK = 0x0001   //* Break
   };
   UsartIO( UART_HandleTypeDef *a_uah, USART_TypeDef *a_us )
     : uah( a_uah ), us( a_us )
    { }
   // virtual ~UsartIO() override;
   // virtual void reset() override;

   // virtual int sendBlock( const char *s, int l ) override;
   virtual int sendBlockSync( const char *s, int l ) override;

   // virtual int recvByte( char *b, int w_tick = 0 ) override;
   virtual int recvBytePoll( char *b, int w_tick = 0 ) override;

   virtual int  setAddrLen( int addrLen UNUSED_ARG ) override { return 0; };
   virtual int  getAddrLen() const override { return 0; };
   virtual int  setAddr( uint32_t addr UNUSED_ARG ) override { return 0; };

   void handleIRQ();
   virtual void task_send() override; // special

   void sendRaw( uint16_t v ) { us->USART_TX_REG = ( v & (uint16_t)0x01FF); };
   int16_t recvRaw() { return (uint16_t)( us->USART_RX_REG & (uint16_t)0x01FF ); };
   void enable()  { us->CR1 |=  CR1_UE;  };
   void disable() { us->CR1 &= ~CR1_UE; };
   void itEnable(  uint32_t it )  { __HAL_UART_ENABLE_IT(  uah, it ); };
   void itDisable( uint32_t it )  { __HAL_UART_DISABLE_IT( uah, it ); };
   void sendBrk() { us->CR1 |= CR1_SBK; };
   bool checkFlag( uint16_t flg ) { return ( us->USART_SR_REG & flg ); };
   void clearFlag( uint16_t flg ) { us->USART_SR_REG &= (uint16_t)~flg; };
   uint16_t getSR() { return us->USART_SR_REG; }
   uint16_t getCR1() { return us->CR1; }
   // ITStatus getITStatus( uint16_t it ) { return USART_GetITStatus( us, it ); };
   // void clearITPendingBit( uint16_t it ) { return USART_ClearITPendingBit( us, it );} ;

  protected:
   UART_HandleTypeDef *uah;
   USART_TypeDef *us;
};

// common declarations
extern "C" {
  void USART1_IRQHandler(void);
  void USART2_IRQHandler(void);
  void USART3_IRQHandler(void);
  void USART4_IRQHandler(void);
  void USART5_IRQHandler(void);
  void USART6_IRQHandler(void);
  void UART7_IRQHandler(void);
  void UART8_IRQHandler(void);
  void task_usart1_send( void *prm UNUSED_ARG );
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
//
#define STD_USART1_IRQ( obj ) void USART1_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART2_IRQ( obj ) void USART2_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART3_IRQ( obj ) void USART3_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART4_IRQ( obj ) void USART4_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART5_IRQ( obj ) void USART5_IRQHandler(void) { obj.handleIRQ(); }
#define STD_USART6_IRQ( obj ) void USART6_IRQHandler(void) { obj.handleIRQ(); }
#define STD_UART7_IRQ( obj )  void UART7_IRQHandler(void)  { obj.handleIRQ(); }
#define STD_UART8_IRQ( obj )  void UART8_IRQHandler(void)  { obj.handleIRQ(); }

#define STD_USART1_SEND_TASK( obj ) STD_COMMON_SEND_TASK( task_usart1_send, obj )
#define STD_USART2_SEND_TASK( obj ) STD_COMMON_SEND_TASK( task_usart2_send, obj )
#define STD_USART3_SEND_TASK( obj ) STD_COMMON_SEND_TASK( task_usart3_send, obj )
#define STD_USART4_SEND_TASK( obj ) STD_COMMON_SEND_TASK( task_usart4_send, obj )
#define STD_USART5_SEND_TASK( obj ) STD_COMMON_SEND_TASK( task_usart5_send, obj )
#define STD_USART6_SEND_TASK( obj ) STD_COMMON_SEND_TASK( task_usart6_send, obj )
#define STD_UART7_SEND_TASK( obj )  STD_COMMON_SEND_TASK( task_uart7_send,  obj )
#define STD_UART8_SEND_TASK( obj )  STD_COMMON_SEND_TASK( task_uart8_send,  obj )
//
#define STD_USART1_RECV_TASK( obj ) STD_COMMON_RECV_TASK( task_usart1_recv, obj )
#define STD_USART2_RECV_TASK( obj ) STD_COMMON_RECV_TASK( task_usart2_recv, obj )
#define STD_USART3_RECV_TASK( obj ) STD_COMMON_RECV_TASK( task_usart3_recv, obj )
#define STD_USART4_RECV_TASK( obj ) STD_COMMON_RECV_TASK( task_usart4_recv, obj )
#define STD_USART5_RECV_TASK( obj ) STD_COMMON_RECV_TASK( task_usart5_recv, obj )
#define STD_USART6_RECV_TASK( obj ) STD_COMMON_RECV_TASK( task_usart6_recv, obj )
#define STD_UART7_RECV_TASK( obj )  STD_COMMON_RECV_TASK( task_uart7_recv,  obj )
#define STD_UART8_RECV_TASK( obj )  STD_COMMON_RECV_TASK( task_uart8_recv,  obj )

#endif
// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include
