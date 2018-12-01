#ifndef _OXC_OUTSTREAM_H
#define _OXC_OUTSTREAM_H

#include <stdint.h>

#include <oxc_miscfun.h>
#include <oxc_outfmt.h>
#include <oxc_io.h>

class OutStream;

class OutStreamFmt {
  public:
   virtual void out( OutStream &os ) const = 0;
};

//* minimal class for outout to given DevOut

class OutStream {
  public:
   OutStream( DevOut * _out ) : out( _out ) {};
   OutStream( const OutStream &r ) = default;
   ~OutStream() { flush(); }
   void flush() { if( out ) { out->flush_out(); } }
   OutStream& operator=( const OutStream &rhs ) = default;
   void append( char rhs ) { if( out ) { out->putc( rhs ); } }
   void add_bitnames( uint32_t b, const BitNames *bn );
   OutStream& operator+=( char rhs ) { append( rhs ); return *this; }
   OutStream& operator<<( char rhs ) { return operator+=( rhs ); }
   OutStream& operator+=( const char *rhs ){ if( out ) { out->puts( rhs ); }; return *this; };
   OutStream& operator<<( const char *rhs ) { return operator+=( rhs ); }
   OutStream& operator+=( int rhs );
   OutStream& operator<<( int rhs )       { return operator+=( rhs ); }
   OutStream& operator<<( unsigned rhs )  { return operator+=( (int)rhs ); }
   OutStream& operator<<( long rhs )      { return operator+=( (int)rhs ); }
   OutStream& operator<<( unsigned long rhs )  { return operator+=( (int)rhs ); }
   OutStream& operator+=( HexInt8  rhs );
   OutStream& operator<<( HexInt8  rhs ) { return operator+=( rhs ); }
   OutStream& operator+=( HexInt16 rhs );
   OutStream& operator<<( HexInt16 rhs ) { return operator+=( rhs ); }
   OutStream& operator+=( HexInt   rhs );
   OutStream& operator<<( HexInt   rhs ) { return operator+=( rhs ); }
   OutStream& operator+=( const FmtInt &rhs );
   OutStream& operator<<( const FmtInt &rhs ) { return operator+=( rhs ); }
   OutStream& operator+=( FixedPoint1 rhs );
   OutStream& operator<<( FixedPoint1 rhs ) { return operator+=( rhs ); }
   OutStream& operator+=( FixedPoint2 rhs );
   OutStream& operator<<( FixedPoint2 rhs ) { return operator+=( rhs ); }
   OutStream& operator+=( FloatMult rhs );
   OutStream& operator<<( FloatMult rhs ) { return operator+=( rhs ); }
   OutStream& operator+=( const BitsStr &rhs );
   OutStream& operator<<( const BitsStr &rhs ) { return operator+=( rhs ); }
   OutStream& operator<<( const OutStreamFmt &rhs ) { rhs.out( *this ); return *this; }
  private:
   DevOut *out = nullptr;
};

// require <oxc_devio.h>
#define STDOUT_os  OutStream os( devio_fds[1] )

#endif
