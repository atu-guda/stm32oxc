#ifndef _OXC_USARTIO_H
#define _OXC_USARTIO_H

#include <oxc_devio.h>
#include <oxc_usartbase.h>



class UsartIO : public DevIO {
  public:
   enum { // CR1 reg bits
     CR1_SBK     = 0x0001,  //* Break
     CR1_RWU     = 0x0002,  //* Recv wakeup
     CR1_RE      = 0x0004,  //* Recv enable
     CR1_TE      = 0x0008,  //* Trans enable
     CR1_IDLEIE  = 0x0010,  //* Idle IRQ enable
     CR1_RXNEIE  = 0x0020,  //* RXNE IRQ enable
     CR1_TCIE    = 0x0040,  //* TC IRQ enable
     CR1_TXEIE   = 0x0080,  //* TXE IRQ enable
     CR1_PEE     = 0x0100,  //* PE IRQ enable
     CR1_PS      = 0x0200,  //* Parity selection:  0 = even,  1 = odd (if PCE)
     CR1_PCE     = 0x0400,  //* Parity control enable
     CR1_WAKE    = 0x0800,  //* Wakeup method: 0 = idle, 1 = Addr mark
     CR1_M       = 0x1000,  //* Word (data) length: 0 = 8, 1 = 9;
     CR1_UE      = 0x2000,  //* Main Enable
     CR1_OVER8   = 0x8000,  //* 0 = oversamling by 16, 1: by 8
   };

   enum { // SR reg bits (ofs=0) *E clear: read SR, read DR
     SR_PE       = 0x0001,  //* Parity error (r)
     SR_FE       = 0x0002,  //* Frame error  (r)
     SR_NF       = 0x0004,  //* Noise detected (r)
     SR_ORE      = 0x0008,  //* Overrun error (r)
     SR_IDLE     = 0x0010,  //* IDLE condition detected (r)
     SR_RXNE     = 0x0020,  //* RX not empty (data to read in DR) (rc_w0)
     SR_TC       = 0x0040,  //* Transmission complete (rc_w0)
     SR_TXE      = 0x0080,  //* TX empty (you can write next) (r)
     SR_LBD      = 0x0100,  //* LIN break detected (rc_w0)
     SR_CTS      = 0x0200,  //* nCTS input toggle (rc_w0)
   };

   UsartIO( UART_HandleTypeDef *a_uah, USART_TypeDef *a_us )
     : uah( a_uah ), us( a_us )
    { }
   // virtual ~UsartIO() override;
   // virtual void reset() override;

   // virtual int write( const char *s, int l ) override;
   virtual int write_s( const char *s, int l ) override;

   // virtual int getc( int w_tick = 0 ) override;
   virtual Chst getc_p( int w_tick = 0 ) override;

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
  SMLRL::SmallRL srl( smallrl_exec ); \
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
