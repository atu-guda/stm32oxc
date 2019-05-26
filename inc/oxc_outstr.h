#ifndef _OXC_OUTSTR_H
#define _OXC_OUTSTR_H

#include <oxc_io.h>

//* Class to use OutStream feature for string buffer
//* Used external buffer, no locks - only for simple local usage

class OutStr: public DevOut {
  public:
   OutStr( char *ext_buf, unsigned buf_sz ) :
     buf( ext_buf ), bsz( buf_sz ) { buf[0] = '\0'; };
   OutStr( const OutStr& rhs ) = delete;
   OutStr& operator=( const OutStr &rhs ) = delete;
   virtual void reset_out() override;
   virtual int  write( const char *s, int l ) override;
   virtual int  puts( const char *s ) override;
   virtual int  putc( char b ) override;
   virtual void flush_out()  override;
   const char* c_str() const { return buf; }
   unsigned size() const { return sz; }
   bool empty() const { return sz == 0; }
   virtual const char* getBuf() const override { return buf; }
   char  operator[]( unsigned i ) const { return (i<bsz) ? buf[i] : '\0'; }
   char& operator[]( unsigned i ) { return (i<bsz) ? buf[i] : fake; }
  protected:
   char *buf;
   unsigned bsz;
   unsigned sz = 0;
   char fake = '\0';
};

#define OSTR(x,sz) char x ## _buf[sz]; OutStr x ## _outstr( x ## _buf, sz ); OutStream x( & x ## _outstr );


#endif

