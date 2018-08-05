#ifndef _OXC_USARTIO_H
#define _OXC_USARTIO_H

#include <oxc_devio.h>
#include <oxc_usartbase.h>



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

   void handleIRQ();
   virtual void on_tick_action_tx() override; // special
   // virtual void on_tick_action_rx() override;
   // virtual void on_tick_action() override;

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
   virtual void start_transmit() override;

  protected:
   UART_HandleTypeDef *uah;
   USART_TypeDef *us;
};

// common declarations
//


#define UART_CONSOLE_DEFINES( dev ) \
  UART_HandleTypeDef uah_console; \
  UsartIO dev_console( &uah_console, dev ); \
  STD_ ## dev ## _IRQ( dev_console );  \
  SmallRL srl( smallrl_exec ); \
  STD_POST_EXEC;

//    STD_ ## dev ## _SEND_TASK( dev_console ); // --

#define SET_UART_AS_STDIO( io ) \
  io.itEnable( UART_IT_RXNE ); \
  io.setOnSigInt( sigint ); \
  srl.setPostExecFun( standart_post_exec ); \
  devio_fds[0] = &io; \
  devio_fds[1] = &io; \
  devio_fds[2] = &io; \
  delay_ms( 10 );

#endif
// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include
