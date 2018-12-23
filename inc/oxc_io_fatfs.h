#ifndef _OXC_IO_FATFS_H
#define _OXC_IO_FATFS_H

#include <oxc_io.h>
#include <ff.h>

class DevOut_FatFS : public DevOut {
  public:
   DevOut_FatFS( const char *fn, BYTE fmode = FA_WRITE | FA_OPEN_ALWAYS );
   DevOut_FatFS( FIL *pf ); // not owning
   ~DevOut_FatFS();
   FIL* getFFile() { return pfile; }
   virtual void reset_out() override;
   virtual int  write( const char *s, int l ) override;
   virtual int  puts( const char *s ) override;
   virtual int  putc( char b ) override;
   virtual void flush_out()  override;
   BYTE getErr() const { return pfile ? (pfile->err) : (BYTE)(FR_INVALID_OBJECT); }
   bool isGood() const { return getErr() == FR_OK; }
  protected:
   FIL ofile;
   FIL *pfile = nullptr;
   bool own;
};

// TODO:
// class DevIn_FatFS {
//   public:
//    virtual void reset_in() override;
//    virtual Chst tryGet() override;
//    virtual unsigned tryGetLine( char *d, unsigned max_len ) override ;
//    virtual Chst getc( int w_tick = 0 ) override;
//    virtual int read( char *s, int l, int w_tick = 0 );
//    virtual void unget( char c ) override;
// };

#endif
