#ifndef _OXC_DEVIO_H
#define _OXC_DEVIO_H

#include <oxc_base.h>

// transmit mode default: waitSync( wait_tx)  + waitFlush
//   maybe:    ASYNC_TX: wait only mutex, if buffer full: return 0, EAGAIN : may be special return type (len,err)
//             ASYNC_MU_MU: 0, EAGAIN if fail to get mutex
//             SYNC: auto waitFlush( ??? )
//   atomic_add?
//     need RingBuf:
//       atomic puts, if available
//       iteration puts, lock only on every char
//     modes: see socket params
// Recv: IRQ->buf( tryPut[s] ), so no atomic, drop
//   default: ASYNC read
//   more default: function callback to console
//   maybe:  SYNC_RX_1: at least 1 + wait
//   maybe:  SYNC_RX_all: all + wait

// portEND_SWITCHING_ISR( wake );
// taskYieldFun();

class DevIO {
  public:
   using OnRecvFun = void (*)( const char *s, int l );
   using SigFun = void (*)( int v );
   enum {
     IBUF_DEF_SIZE = 128, //* low-level transmit buffer size
     OBUF_DEF_SIZE = 128  //* low-level receive buffer size
   };

   DevIO( unsigned ibuf_sz = IBUF_DEF_SIZE, unsigned obuf_sz = OBUF_DEF_SIZE )
     : ibuf( ibuf_sz ),
       obuf( obuf_sz )
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
   int sendByte( char b )     { return sendBlock( &b, 1 ); };
   int sendByteSync( char b ) { return sendBlockSync( &b, 1 ); };

   virtual int recvByte( char *s, int w_tick = 0 ); // TODO: Chst
   virtual int recvBytePoll( char *s, int w_tick = 0 ) = 0;
   virtual int recvBlock( char *s, int l, int w_tick = 0 ); // w_tick - for every
   virtual int recvBlockPoll( char *s, int l, int w_tick = 0 );
   virtual void setOnRecv( OnRecvFun a_onRecv ) { onRecv = a_onRecv; };
   virtual void setOnSigInt( SigFun a_onSigInt ) { onSigInt = a_onSigInt; };

   virtual int task_send();
   virtual int task_recv();
   virtual int task_io();
   void charFromIrq( char c ); // called from IRQ!
   void charsFromIrq( const char *s, int l ); //  called from IRQ!

  protected:
   // TODO: open mode + flags: block_send...
   RingBuf ibuf;
   RingBuf obuf;
   OnRecvFun onRecv = nullptr;
   SigFun onSigInt = nullptr;
   int err = 0;
   int wait_tx = 1500;
   int wait_rx = 1500;
   bool on_transmit = false;
};

// fd - ala
const int DEVIO_MAX = 16;      //* Maximum number of devios
const int DEVIO_STDIN_NO = 0;  //* Number of stdin
const int DEVIO_STDOUT_NO = 1; //* Number of stdout
const int DEVIO_STDERR_NO = 2; //* Number of stderr
extern DevIO *devio_fds[DEVIO_MAX];

int recvByte( int fd, char *s, int w_tick = 0 );
int sendBlock( int fd, const char *s, int l );
int pr( const char *s, int fd = 1 );
int prl( const char *s, unsigned l, int fd = 1 );
int prl1( const char *s, unsigned l ); // fd == 1 for used as flush funcs

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

extern "C" { // TODO: move to base
// defines from newlib: hidden by some macros
int  asprintf( char **__restrict, const char *__restrict, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
int  vasprintf( char **, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
int  asiprintf( char **, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
char *  asniprintf( char *, size_t *, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 3, 4)));
char *  asnprintf( char *__restrict, size_t *__restrict, const char *__restrict, ... )
               _ATTRIBUTE( (__format__( __printf__, 3, 4)));
int  diprintf( int, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
int  fiprintf( FILE *, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
int  fiscanf( FILE *, const char *, ... )
               _ATTRIBUTE( (__format__( __scanf__, 2, 3)));
int  iprintf( const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 1, 2)));
int  iscanf( const char *, ... )
               _ATTRIBUTE( (__format__( __scanf__, 1, 2)));
int  siprintf( char *, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 2, 3)));
int  siscanf( const char *, const char *, ... )
               _ATTRIBUTE( (__format__( __scanf__, 2, 3)));
int  sniprintf( char *, size_t, const char *, ... )
               _ATTRIBUTE( (__format__( __printf__, 3, 4)));
int  vasiprintf( char **, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
char *  vasniprintf( char *, size_t *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 3, 0)));
char *  vasnprintf( char *, size_t *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 3, 0)));
int  vdiprintf( int, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
int  vfiprintf( FILE *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
int  vfiscanf( FILE *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __scanf__, 2, 0)));
int  viprintf( const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 1, 0)));
int  viscanf( const char *, __VALIST )
               _ATTRIBUTE( (__format__( __scanf__, 1, 0)));
int  vsiprintf( char *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 2, 0)));
int  vsiscanf( const char *, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __scanf__, 2, 0)));
int  vsniprintf( char *, size_t, const char *, __VALIST )
               _ATTRIBUTE( (__format__( __printf__, 3, 0)));
};

#endif

