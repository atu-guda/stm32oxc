#ifndef _OXC_DEVIO_H
#define _OXC_DEVIO_H

#include <oxc_rtosqueue.h>


class DevIO {
  public:
   using OnRecvFun = void (*)( const char *s, int l );
   using SigFun = void (*)( int v );
   enum {
     IBUF_SZ = 128,     //* Input queue  size
     OBUF_SZ = 128,     //* Output queue size
     TX_BUF_SIZE = 256, //* low-level transmit buffer size
     RX_BUF_SIZE = 256  //* low-level receive buffer size, buffer itself - only if required
   };

   DevIO( unsigned ibuf_sz = IBUF_SZ, unsigned obuf_sz = OBUF_SZ )
     : ibuf( ibuf_sz, 1 ),
       obuf( obuf_sz, 1 )
    {};
   virtual ~DevIO();
   virtual void reset();
   virtual int getErr() const { return err; }
   void setWaitTx( int tx ) { wait_tx = tx; }
   void setWaitRx( int rx ) { wait_rx = rx; }

   virtual int sendBlock( const char *s, int l );
   virtual int sendBlockSync( const char *s, int l ) = 0;
   virtual int sendStr( const char *s );
   virtual int sendStrSync( const char *s );
   int sendByte( char b ) { return sendBlock( &b, 1 ); };
   int sendByteSync( char b ) { return sendBlockSync( &b, 1 ); };
   int sendInt16( int16_t v ) { return sendBlock( (const char*)(&v), sizeof(int16_t) ); };
   int sendInt16Sync( int16_t v ) { return sendBlockSync( (const char*)(&v), sizeof(int16_t) ); };
   int sendInt32( int32_t v ) { return sendBlock( (const char*)(&v), sizeof(int32_t) ); };
   int sendInt32Sync( int32_t v ) { return sendBlockSync( (const char*)(&v), sizeof(int32_t) ); };

   virtual int recvByte( char *s, int w_tick = 0 );
   virtual int recvBytePoll( char *s, int w_tick = 0 ) = 0;
   virtual int recvBlock( char *s, int l, int w_tick = 0 ); // w_tick - for every
   virtual int recvBlockPoll( char *s, int l, int w_tick = 0 );
   virtual void setOnRecv( OnRecvFun a_onRecv ) { onRecv = a_onRecv; };
   virtual void setOnSigInt( SigFun a_onSigInt ) { onSigInt = a_onSigInt; };

   virtual void task_send();
   virtual void task_recv();
   void charsFromIrq( const char *s, int l ); // virtual?

   virtual int  setAddrLen( int addrLen ) = 0;
   virtual int  getAddrLen() const = 0;
   virtual int  setAddr( uint32_t addr ) = 0;
   // void initIRQ( uint8_t ch, uint8_t prio ); // TODO:? store ch
  protected:
   // TODO: open mode + flags
   RtosQueue ibuf;
   RtosQueue obuf;
   OnRecvFun onRecv = nullptr;
   SigFun onSigInt = nullptr;
   int err = 0;
   int wait_tx = 1500;
   int wait_rx = 1500;
   bool on_transmit = false;
   char tx_buf[TX_BUF_SIZE];
};

// fd - ali
const int DEVIO_MAX = 16;      //* Maximum number of devios
const int DEVIO_STDIN_NO = 0;  //* Number of stdin
const int DEVIO_STDOUT_NO = 1; //* Number of stdout
const int DEVIO_STDERR_NO = 2; //* Number of stderr
extern DevIO *devio_fds[DEVIO_MAX];

int recvByte( int fd, char *s, int w_tick = 0 );
int sendBlock( int fd, const char *s, int l );
int pr( const char *s, int fd = 1 );
int prl( const char *s, int l, int fd = 1 );

#define STD_COMMON_RECV_TASK( name, obj ) \
  void name( void *prm UNUSED_ARG ) { while(1) {  obj.task_recv(); }  vTaskDelete(0); }
#define STD_COMMON_SEND_TASK( name, obj ) \
  void name( void *prm UNUSED_ARG ) { while(1) {  obj.task_send(); }  vTaskDelete(0); }

class DevNull : public DevIO {
  public:
   DevNull() {};
   virtual int sendBlock( const char *s UNUSED_ARG, int l ) override { return l; };
   virtual int sendBlockSync( const char *s UNUSED_ARG, int l ) override { return l; };

   virtual int recvByte( char *b UNUSED_ARG, int w_tick UNUSED_ARG = 0 ) override { return 0; };
   virtual int recvBytePoll( char *b UNUSED_ARG, int w_tick UNUSED_ARG = 0 ) override { return 0; };

   virtual int  setAddrLen( int addrLen UNUSED_ARG ) override { return 0; };
   virtual int  getAddrLen() const override { return 0; };
   virtual int  setAddr( uint32_t addr UNUSED_ARG ) override { return 0; };
};



#endif

