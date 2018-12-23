#include <oxc_io_fatfs.h>

DevOut_FatFS::DevOut_FatFS( const char *fn, BYTE fmode )
  : own( true )
{
  f_open( &ofile, fn, fmode );
  if( ofile.err == FR_OK ) {
    pfile = &ofile;
  }
}

DevOut_FatFS::DevOut_FatFS( FIL *pf )
  : pfile( pf ), own( false )
{
}

DevOut_FatFS::~DevOut_FatFS()
{
  if( own && pfile ) {
    f_close( pfile );
    pfile = nullptr;
  }
}


void DevOut_FatFS::reset_out()
{
  if( pfile ) {
    f_sync( pfile );
  }
}

int  DevOut_FatFS::write( const char *s, int l )
{
  if( !pfile ) {
    return -1;
  }
  UINT bw;
  auto err = f_write ( pfile, s, l, &bw );
  if( err == FR_OK ) {
    return bw;
  };
  return -1;
}

int  DevOut_FatFS::puts( const char *s )
{
  if( ! pfile ) {
    return 0;
  }
  return f_puts( s, pfile );
}

int  DevOut_FatFS::putc( char b )
{
  if( ! pfile ) {
    return 0;
  }
  return f_putc( b, pfile );
}

void DevOut_FatFS::flush_out()
{
  reset_out();
}

