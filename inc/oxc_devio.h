#ifndef _OXC_DEVIO_H
#define _OXC_DEVIO_H

#include <oxc_ringbuf.h>
#include <oxc_io.h>


class DevIO {
  public:
   using OnRecvFun = void (*)( const char *s, int l );
   using SigFun = void (*)( int v );
   enum {
     TX_BUF_SIZE = 256, //* low-level transmit buffer size
     RX_BUF_SIZE = 128  //* low-level receive buffer size
   };

   DevIO( unsigned ibuf_sz = RX_BUF_SIZE, unsigned obuf_sz = TX_BUF_SIZE );
   virtual ~DevIO();
   virtual void reset();
   virtual int getErr() const { return err; }
   void setWaitTx( int tx ) { wait_tx = tx; }
   void setWaitRx( int rx ) { wait_rx = rx; }
   void setHandleCbreak( bool h ) { handle_cbreak = h; }

   virtual int sendBlock( const char *s, int l );
   virtual int sendBlockSync( const char *s, int l ) = 0;
   virtual int sendStr( const char *s );
   virtual int sendStrSync( const char *s );
   int sendByte( char b ) { return sendBlock( &b, 1 ); };
   int sendByteSync( char b ) { return sendBlockSync( &b, 1 ); };

   Chst tryGet() { return ibuf.tryGet(); }
   virtual int recvByte( char *s, int w_tick = 0 );
   virtual int recvBytePoll( char *s, int w_tick = 0 ) = 0;
   virtual int recvBlock( char *s, int l, int w_tick = 0 ); // w_tick - for every
   virtual int recvBlockPoll( char *s, int l, int w_tick = 0 );
   virtual void setOnRecv( OnRecvFun a_onRecv ) { onRecv = a_onRecv; };
   virtual void setOnSigInt( SigFun a_onSigInt ) { onSigInt = a_onSigInt; };

   virtual void on_tick_action_tx();
   virtual void on_tick_action_rx();
   virtual void on_tick_action();
   void charFromIrq( char c );
   void charsFromIrq( const char *s, int l ); // virtual?
   int wait_eot( int w = 0 ); // w=0 means forewer, 1 - ok 0 - overtime

   virtual void start_transmit() {};
   void testCbreak( char c ); // called from funcs, called from IRQ!

  protected:
   // TODO: open mode + flags
   RingBuf ibuf;
   RingBuf obuf;
   OnRecvFun onRecv = nullptr;
   SigFun onSigInt = nullptr;
   int err = 0;
   int wait_tx = 1500;
   int wait_rx = 1500;
   bool on_transmit = false;
   bool handle_cbreak = true;
};

// fd - ala
const int DEVIO_MAX = 16;      //* Maximum number of devios
const int DEVIO_STDIN_NO = 0;  //* Number of stdin
const int DEVIO_STDOUT_NO = 1; //* Number of stdout
const int DEVIO_STDERR_NO = 2; //* Number of stderr
extern DevIO *devio_fds[DEVIO_MAX];

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

};

#endif

