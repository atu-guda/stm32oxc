#ifndef _OXC_OUTSTREAM_H
#define _OXC_OUTSTREAM_H

#include <oxc_outfmt.h>
#include <oxc_io.h>


//* minimal class for output to given DevOut

class OutStream {
  public:
   explicit OutStream( DevOut* _out ) : out( _out ) {};
   OutStream( const OutStream &r ) = default;
   ~OutStream() { flush(); }
   void setOut( DevOut* _out ) { out = _out ; };
   DevOut* getOut() { return out; }
   const char* getBuf() const { return out ? out->getBuf() : nullptr; }
   void flush() { if( out ) { out->flush_out(); } }
   void reset_out() { if( out ) { out->reset_out(); } }
   OutStream& operator=( const OutStream &rhs ) = default;
   void append( char rhs ) { if( out ) { out->putc( rhs ); } }
   void append( const char *rhs ) { if( out ) { out->puts( rhs ); } }
   void add_bitnames( uint32_t b, const BitNames *bn );
   OutStream& operator+=( char rhs )        { append( rhs ); return *this; }
   OutStream& operator<<( char rhs )        { append( rhs ); return *this; }
   OutStream& operator+=( const char *rhs ) { append( rhs ); return *this; }
   OutStream& operator<<( const char *rhs ) { append( rhs ); return *this; }
   OutStream& operator+=( int rhs );
   OutStream& operator<<( int rhs )       { return operator+=( rhs ); }
   OutStream& operator<<( unsigned rhs )  { return operator+=( (int)rhs ); }
   OutStream& operator<<( long rhs )      { return operator+=( (int)rhs ); }
   OutStream& operator<<( unsigned long rhs )  { return operator+=( (int)rhs ); }
   OutStream& operator<<( const OutStreamFmt &rhs ) { rhs.out( *this ); return *this; }
  private:
   DevOut *out = nullptr;
};

extern OutStream std_out;


#endif
