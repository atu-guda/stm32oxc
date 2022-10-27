#ifndef _OXC_DEVIO_H
#define _OXC_DEVIO_H

#include <oxc_ringbuf.h>
#include <oxc_io.h>

const int DEVIO_MAX = 16;      //* Maximum number of devios

class DevIO : public DevOut, public DevIn {
  public:
   using OnRecvFun = void (*)( const char *s, int l );
   using SigFun = void (*)( int v );
   enum {
     TX_BUF_SIZE = 256, //* low-level transmit buffer size
     RX_BUF_SIZE = 128 //* low-level receive buffer size
   };

   DevIO( unsigned ibuf_sz = RX_BUF_SIZE, unsigned obuf_sz = TX_BUF_SIZE );
   virtual ~DevIO();
   virtual void reset();
   virtual int getErr() const { return err; }
   void setWaitTx( int tx ) { wait_tx = tx; }
   void setWaitRx( int rx ) { wait_rx = rx; }
   void setHandleCbreak( bool h ) { handle_cbreak = h; }

   virtual void reset_out() override { obuf.reset(); }
   virtual int write( const char *s, int l ) override;
   virtual int write_s( const char *s, int l ) = 0;
   virtual int puts( const char *s ) override;
   virtual int puts_s( const char *s );
   int putc( char b )  override { return write( &b, 1 ); };
   int putc_s( char b ) { return write_s( &b, 1 ); };
   int wait_eot( int w = 0 ); // w=0 means forever, 1 - ok 0 - overtime
   virtual void flush_out() override { wait_eot( wait_tx ); };
   virtual const char* getBuf() const override { return obuf.getBuf(); }

   virtual void reset_in() override { ibuf.reset(); }
   virtual Chst tryGet() override { return ibuf.tryGet(); }
   virtual unsigned tryGetLine( char *d, unsigned max_len ) override { return ibuf.tryGetLine( d, max_len ); } ;
   virtual Chst getc( int w_tick = 0 ) override;
   virtual Chst getc_p( int w_tick = 0 ) = 0;
   virtual int read( char *s, int l, int w_tick = 0 ) override; // w_tick - for every
   virtual int read_poll( char *s, int l, int w_tick = 0 );
   void unget( char c ) override { ibuf.tryPut( c ); }
   void setOnRecv( OnRecvFun a_onRecv ) { onRecv = a_onRecv; };
   void setOnSigInt( SigFun a_onSigInt ) { onSigInt = a_onSigInt; };
   virtual const char* getInBuf() const override { return ibuf.getBuf(); }
   virtual unsigned getInBufSize() const override { return ibuf.size(); }

   virtual void on_tick_action_tx();
   virtual void on_tick_action_rx();
   virtual void on_tick_action();
   static void tick_actions_all();
   void charFromIrq( char c );
   void charsFromIrq( const char *s, int l ); // virtual?

   virtual void start_transmit() {};
   void testCbreak( char c ); // called from function, called from IRQ!

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
   bool ready_transmit = true;
   static DevIO* devios[DEVIO_MAX];
};


// fd - ala
const int DEVIO_STDIN_NO = 0;  //* Number of stdin
const int DEVIO_STDOUT_NO = 1; //* Number of stdout
const int DEVIO_STDERR_NO = 2; //* Number of stderr
extern DevIO *devio_fds[DEVIO_MAX];


class DevNull : public DevIO {
  public:
   DevNull() {};
   virtual int write( const char *s UNUSED_ARG, int l ) override { return l; };
   virtual int write_s( const char *s UNUSED_ARG, int l ) override { return l; };

   virtual Chst getc( int w_tick UNUSED_ARG = 0 ) override { return Chst( '\0', Chst::st_empty ); };
   virtual Chst getc_p( int w_tick UNUSED_ARG = 0 ) override { return Chst( '\0', Chst::st_empty ); };

};

#endif

