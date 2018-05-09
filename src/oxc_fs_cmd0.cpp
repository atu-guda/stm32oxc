#include <string.h>

#include <ff.h>
#include <fatfs.h>

#include <oxc_fs_cmd0.h>

CmdInfo CMDINFO_MOUNT { "mount", 'M', cmd_mount, " [path] - mount FAT filesystem"  };
CmdInfo CMDINFO_UMOUNT { "umount", 'U', cmd_umount, " - umount FAT filesystem"  };
CmdInfo CMDINFO_FSINFO { "fsinfo", 'I', cmd_fsinfo, " info about FAT filesystem"  };
CmdInfo CMDINFO_LS { "ls", 0, cmd_ls, " [path] - list directory contents"  };
CmdInfo CMDINFO_CAT { "cat", 0, cmd_cat, " path [max [noout]] - output file contents to stdout"  };
CmdInfo CMDINFO_APPSTR { "appstr", 0, cmd_appstr, " file string  - append string to file"  };
CmdInfo CMDINFO_WBLOCKS { "wblocks", 'W', cmd_wblocks, " file [n_blocks]  - write blocks to file"  };
CmdInfo CMDINFO_RM { "rm", 0, cmd_rm, " file - remove file"  };

char fspath[fspath_sz];
extern FATFS fs; // from main?
extern uint8_t sd_buf[512];

int cmd_mount( int argc, const char * const * argv )
{
  if( argc > 1 ) {
    strncpy( fspath, argv[1], sizeof( fspath ) - 1 );
  }
  pr( "Try to mount \"" ); pr( fspath ); pr( "\""  NL);
  FRESULT fr = f_mount( &fs, fspath, 1 );

  return fr;
}

int cmd_umount( int argc, const char * const * argv )
{
  pr( "Try to umount \"" ); pr( fspath ); pr( "\""  NL);
  FRESULT fr = f_mount( nullptr, fspath, 1 );

  return fr;
}

int cmd_ls( int argc, const char * const * argv )
{
  const char *dname = "/";
  if( argc > 1 ) {
    dname = argv[1];
  }
  pr( "dir \"" ); pr( dname ); pr( "\"" NL );

  DIR dir;
  FILINFO finfo;
  FRESULT r = f_opendir( &dir, dname );
  if( r == FR_OK ) {
    while( ( r = f_readdir( &dir, &finfo ) )  ==  FR_OK ) {
      if( ! finfo.fname[0] ) { // end of dir
        break;
      }
      pr( finfo.fname );
      if( finfo.fattrib & AM_DIR ) {
        pr( "/" );
      }
      pr( " " ); pr_d( finfo.fsize );
      pr( NL );
    }
    f_closedir( &dir );
  } else {
    pr( "f_opendir error: " ); pr_d( r ); pr( NL );
  }

  return r;
}

int cmd_fsinfo( int argc, const char * const * argv )
{
  const char *fsn = "";
  if( argc > 1 ) {
    fsn = argv[1];
  }
  pr( "fsinfo about \"" ); pr( fsn ); pr( "\"" NL "fs_type= " ); pr_d( fs.fs_type ); pr( NL );
  pr( "drv= " ); pr_d( fs.drv ); pr( NL );
  pr( "csize= " ); pr_d( fs.csize ); pr( NL );
  pr( "n_fats= " ); pr_d( fs.n_fats ); pr( NL );
  pr( "wflag= " ); pr_d( fs.wflag ); pr( NL );
  pr( "fsi_flag= " ); pr_d( fs.fsi_flag ); pr( NL );
  pr( "id= " ); pr_d( fs.id ); pr( NL );

  char vol_buf[32]; vol_buf[0] = '\0';
  uint32_t vsn = 0, n_f_clust = 0;
  FATFS *lfs;
  if( fs.fs_type > 0 ) {
    f_getlabel( fsn, vol_buf, &vsn );
    pr( "volume_label: \"" ); pr( vol_buf ); pr( "\" vsn=" ); pr_d( vsn ); pr( NL );
    f_getfree( fsn, &n_f_clust, &lfs );
    pr( "free_clust: " ); pr_d( n_f_clust ); pr( " kB:" ); pr_d( n_f_clust * fs.csize / 2 ) ;pr( NL );
  }

  return 0;
}

int cmd_cat( int argc, const char * const * argv )
{
  const char *fn = "";
  if( argc > 1 ) {
    fn = argv[1];
  }
  uint32_t max_sz = arg2long_d( 2, argc, argv, 1024, 1, 0x7FFFFFFF );
  bool do_out = ! arg2long_d( 3, argc, argv, 0, 1, 1 );
  pr( "cat file \"" ); pr( fn ); pr( "\"  max_sz= " ); pr_d( max_sz ); pr( NL );

  unsigned nr = 0, to_read, was_read; // number of read bytes
  FIL f;
  FRESULT r = f_open( &f, fn, FA_READ );
  TickType_t tc0 = xTaskGetTickCount(), tc1 = tc0;
  if( r == FR_OK ) {
    while( nr < max_sz ) { // or break
      to_read = max_sz - nr;
      if( to_read > sizeof( sd_buf ) ) {
        to_read = sizeof( sd_buf );
      }
      r = f_read( &f, sd_buf, to_read, &was_read );
      // pr( " to_read= " ); pr_d( to_read ); pr( " was_read= " ); pr_d( was_read ); pr( " r= " ); pr_d( r ); pr( NL );
      if( r != FR_OK || was_read < 1 ) {
        break;
      }
      if( do_out ) {
        sendBlock( 1, (const char*)sd_buf, was_read );
      }
      nr += was_read;
    }
    f_close( &f );
    tc1 = xTaskGetTickCount();
  } else {
    pr( "f_open error: " ); pr_d( r ); pr( NL );
  }

  pr( NL "cat end, read " ); pr_d( nr);
  pr( " bytes, r =  " ); pr_d( r );
  pr( " time =  " ); pr_d( tc1 - tc0 ); pr( NL );
  return r;
}

int cmd_appstr( int argc, const char * const * argv )
{
  if( argc < 3 ) {
    pr( "Error: need filename and string" NL );
    return 1;
  }

  const char *fn = argv[1], *s = argv[2];
  unsigned  was_wr = 0;
  FIL f;
  FRESULT r = f_open( &f, fn, FA_WRITE | FA_OPEN_ALWAYS );
  // float zz = 0.123456;
  if( r == FR_OK ) {
    r = f_lseek( &f, f_size(&f) );
    was_wr = f_puts( s, &f );
    // f_printf( &f, "more %d %f string\r\n", f.fsize, zz );
    f_close( &f );
  } else {
    pr( "f_open error: " ); pr_d( r ); pr( NL );
  }

  pr( NL "appstr end, r= " ); pr_d( r ); pr( " was_wr= "); pr_d( was_wr ); pr( NL );
  return r;
}


int cmd_wblocks( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Error: need filename [n_sect]" NL );
    return 1;
  }
  uint32_t n_sect = arg2long_d( 2, argc, argv, 1, 1, 1024*2048 );

  const char *fn = argv[1];
  unsigned  was_wr = 0, w;
  memset( sd_buf, '1', sizeof( sd_buf ) );
  FIL f;
  FRESULT r = f_open( &f, fn, FA_WRITE | FA_OPEN_ALWAYS );
  TickType_t tc0 = xTaskGetTickCount(), tc1 = tc0;
  if( r == FR_OK ) {
    for( unsigned i=0; i < n_sect; ++i ) {
      r = f_write( &f, sd_buf, sizeof( sd_buf ), &w );
      was_wr += w;
      if( r != FR_OK ) {
        break;
      }
    }
    tc1 = xTaskGetTickCount();
    f_close( &f );
  } else {
    pr( "f_open error: " ); pr_d( r ); pr( NL );
  }

  pr( NL "wblocks end, r= " ); pr_d( r ); pr( " was_wr= "); pr_d( was_wr );
  pr( " time =  " ); pr_d( tc1 - tc0 ); pr( NL );
  return r;
}

int cmd_rm( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Error: need filename to delete" NL );
    return 1;
  }

  const char *fn = argv[1];
  FRESULT r = f_unlink( fn );
  if( r != FR_OK ) {
    pr( "f_unlink error: " ); pr_d( r ); pr( NL );
  }

  return r;
}



