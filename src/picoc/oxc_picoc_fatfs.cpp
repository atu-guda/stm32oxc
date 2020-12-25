#include <oxc_auto.h>
#include <cerrno>

#include <ff.h>

#include <oxc_picoc_interpreter.h>
#include <oxc_picoc_reghelpers.h>

using namespace std;



#pragma GCC diagnostic ignored "-Wunused-parameter"

// TODO: new/delete
const unsigned ff_max_open = 4;
FIL ff_open_files[ff_max_open];
bool ff_fd_busy[ff_max_open];

// FRESULT f_truncate (FIL* fp);										#<{(| Truncate the file |)}>#
// FRESULT f_sync (FIL* fp);											#<{(| Flush cached data of the writing file |)}>#
// FRESULT f_opendir (DIR* dp, const TCHAR* path);						#<{(| Open a directory |)}>#
// FRESULT f_closedir (DIR* dp);										#<{(| Close an open directory |)}>#
// FRESULT f_readdir (DIR* dp, FILINFO* fno);							#<{(| Read a directory item |)}>#
// FRESULT f_findfirst (DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern);	#<{(| Find first file |)}>#
// FRESULT f_findnext (DIR* dp, FILINFO* fno);							#<{(| Find next file |)}>#
// FRESULT f_mkdir (const TCHAR* path);								#<{(| Create a sub directory |)}>#
// FRESULT f_unlink (const TCHAR* path);								#<{(| Delete an existing file or directory |)}>#
// FRESULT f_rename (const TCHAR* path_old, const TCHAR* path_new);	#<{(| Rename/Move a file or directory |)}>#
// FRESULT f_stat (const TCHAR* path, FILINFO* fno);					#<{(| Get file status |)}>#
// FRESULT f_chmod (const TCHAR* path, BYTE attr, BYTE mask);			#<{(| Change attribute of a file/dir |)}>#
// FRESULT f_utime (const TCHAR* path, const FILINFO* fno);			#<{(| Change timestamp of a file/dir |)}>#
// FRESULT f_chdir (const TCHAR* path);								#<{(| Change current directory |)}>#
// FRESULT f_chdrive (const TCHAR* path);								#<{(| Change current drive |)}>#
// FRESULT f_getcwd (TCHAR* buff, UINT len);							#<{(| Get current directory |)}>#
// FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs);	#<{(| Get number of free clusters on the drive |)}>#
// FRESULT f_getlabel (const TCHAR* path, TCHAR* label, DWORD* vsn);	#<{(| Get volume label |)}>#
// FRESULT f_setlabel (const TCHAR* label);							#<{(| Set volume label |)}>#
// FRESULT f_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	#<{(| Forward data to the stream |)}>#
// FRESULT f_expand (FIL* fp, FSIZE_t fsz, BYTE opt);					#<{(| Allocate a contiguous block to the file |)}>#
// FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt);			#<{(| Mount/Unmount a logical drive |)}>#
// FRESULT f_mkfs (const TCHAR* path, const MKFS_PARM* opt, void* work, UINT len);	#<{(| Create a FAT volume |)}>#
// FRESULT f_setcp (WORD cp);											#<{(| Set current code page |)}>#
// int f_printf (FIL* fp, const TCHAR* str, ...);						#<{(| Put a formatted string to the file |)}>#
//
// #define f_eof(fp) ((int)((fp)->fptr == (fp)->obj.objsize))
// #define f_error(fp) ((fp)->err)
// #define f_tell(fp) ((fp)->fptr)
// #define f_size(fp) ((fp)->obj.objsize)
// #define f_rewind(fp) f_lseek((fp), 0)
// #define f_rewinddir(dp) f_readdir((dp), 0)
// #define f_rmdir(path) f_unlink(path)
// #define f_unmount(path) f_mount(0, path, 0)


const int C_FA_READ          = FA_READ;
const int C_FA_WRITE         = FA_WRITE;
const int C_FA_OPEN_EXISTING = FA_OPEN_EXISTING;
const int C_FA_CREATE_NEW    = FA_CREATE_NEW;
const int C_FA_CREATE_ALWAYS = FA_CREATE_ALWAYS;
const int C_FA_OPEN_ALWAYS   = FA_OPEN_ALWAYS;
const int C_FA_OPEN_APPEND   = FA_OPEN_APPEND;

void C_f_open( PICOC_FUN_ARGS );
void C_f_open( PICOC_FUN_ARGS )
{
  const char *fn = (const char*)ARG_0_PTR;
  int mode = ARG_1_INT;
  int found_idx = -1;
  for( unsigned i=0; i< ff_max_open; ++i ) {
    if( ff_fd_busy[i] == false ) {
      found_idx = i;
      break;
    }
  }

  if( found_idx < 0 ) {
    RV_INT = -1;
    errno = EMFILE;
    return;
  }

  FRESULT fr = f_open( &ff_open_files[found_idx], fn, mode );
  if( fr != FR_OK ) {
    RV_INT = -1;
    errno = fr;
    return;
  }

  ff_fd_busy[found_idx] = true; // RACE?
  RV_INT = found_idx;
}

#define FF_CHECK_FD(fd) \
  if( fd < 0 || (unsigned)fd >= ff_max_open || !ff_fd_busy[fd] ) { \
    errno = EBADF; \
    RV_INT = -1; \
    return; \
  } \

#define CUR_FILE &ff_open_files[fd]

void C_f_close( PICOC_FUN_ARGS );
void C_f_close( PICOC_FUN_ARGS )
{
  int fd = ARG_0_INT;
  FF_CHECK_FD( fd );

  FRESULT fr = f_close( CUR_FILE );
  if( fr != FR_OK ) {
    errno = fr;
    RV_INT = -1;
    return;
  }

  ff_fd_busy[fd] = false; // RACE?
  RV_INT = 0;
}


void C_f_read( PICOC_FUN_ARGS );
void C_f_read( PICOC_FUN_ARGS )
{
  int fd    = ARG_0_INT;
  void *buf = ARG_1_PTR;
  int r     = ARG_2_INT;
  FF_CHECK_FD( fd );

  unsigned rr = 0;
  FRESULT fr = f_read( CUR_FILE, buf, r, &rr );
  if( fr != FR_OK ) {
    errno = fr;
    RV_INT = -1;
    return;
  }

  RV_INT = rr;
}


void C_f_write( PICOC_FUN_ARGS );
void C_f_write( PICOC_FUN_ARGS )
{
  int fd    = ARG_0_INT;
  void *buf = ARG_1_PTR;
  int w     = ARG_2_INT;
  FF_CHECK_FD( fd );

  unsigned ww = 0;
  FRESULT fr = f_write( CUR_FILE, buf, w, &ww );
  if( fr != FR_OK ) {
    errno = fr;
    RV_INT = -1;
    return;
  }

  RV_INT = ww;
}

void C_f_lseek( PICOC_FUN_ARGS );
void C_f_lseek( PICOC_FUN_ARGS )
{
  int fd    = ARG_0_INT;
  int pos   = ARG_1_INT; // not uint64_t for picoc
  FF_CHECK_FD( fd );

  FRESULT fr = f_lseek( CUR_FILE, pos );
  if( fr != FR_OK ) {
    errno = fr;
  }
}


void C_f_putc( PICOC_FUN_ARGS );
void C_f_putc( PICOC_FUN_ARGS )
{
  char  c   = (char)ARG_0_INT;
  int fd    = ARG_1_INT;
  FF_CHECK_FD( fd );
  f_putc( c, CUR_FILE );
}

void C_f_puts( PICOC_FUN_ARGS );
void C_f_puts( PICOC_FUN_ARGS )
{
  char *s   = (char*)ARG_0_PTR;
  int fd    = ARG_1_INT;
  FF_CHECK_FD( fd );
  f_puts( s, CUR_FILE );
}

void C_f_gets( PICOC_FUN_ARGS );
void C_f_gets( PICOC_FUN_ARGS )
{
  char *buf = (char*)ARG_0_PTR;
  int len   = ARG_1_INT;
  int fd    = ARG_2_INT;
  FF_CHECK_FD( fd );
  char *b = f_gets( buf, len, CUR_FILE );
  RV_INT = b ? strlen( b ) : 0;
}

void C_f_eof( PICOC_FUN_ARGS );
void C_f_eof( PICOC_FUN_ARGS )
{
  int fd    = ARG_0_INT;
  FF_CHECK_FD( fd );
  RV_INT = f_eof( CUR_FILE );
}

void C_f_size( PICOC_FUN_ARGS );
void C_f_size( PICOC_FUN_ARGS )
{
  int fd    = ARG_0_INT;
  FF_CHECK_FD( fd );
  RV_INT = f_size( CUR_FILE );
}

void C_f_error( PICOC_FUN_ARGS );
void C_f_error( PICOC_FUN_ARGS )
{
  int fd    = ARG_0_INT;
  FF_CHECK_FD( fd );
  RV_INT = f_error( CUR_FILE );
}


struct LibraryFunction oxc_picoc_fatfs_Functions[] =
{
  { C_f_open,       "int f_open( char*, int mode );" },
  { C_f_close,      "int f_close( int fd );" },
  { C_f_read,       "int f_read( int fd, void *buf, int r );" },
  { C_f_write,      "int f_write( int fd, void *buf, int w );" },
  { C_f_lseek,      "int f_lseek( int fd, int pos );" },
  { C_f_putc,       "void f_putc( char c, int fd );" },
  { C_f_puts,       "void f_puts( char *s, int fd );" },
  { C_f_gets,       "int f_gets( char *buf, int len, int fd );" },
  { C_f_eof,        "int f_eof( int fd );" },
  { C_f_size,       "int f_size( int fd );" },
  { C_f_error,      "int f_error( int fd );" },
  { NULL,            NULL }
};

void oxc_picoc_fatfs_SetupFunc( Picoc *pc );
void oxc_picoc_fatfs_SetupFunc( Picoc *pc )
{
}

void oxc_picoc_fatfs_init( Picoc *pc );
void oxc_picoc_fatfs_init( Picoc *pc )
{
  for( auto &v : ff_fd_busy ) { v = false; }
  VariableDefinePlatformVar( pc, nullptr, "FA_READ"         ,  &pc->IntType,  (union AnyValue *)(&C_FA_READ         ), FALSE );
  VariableDefinePlatformVar( pc, nullptr, "FA_WRITE"        ,  &pc->IntType,  (union AnyValue *)(&C_FA_WRITE        ), FALSE );
  VariableDefinePlatformVar( pc, nullptr, "FA_OPEN_EXISTING",  &pc->IntType,  (union AnyValue *)(&C_FA_OPEN_EXISTING), FALSE );
  VariableDefinePlatformVar( pc, nullptr, "FA_CREATE_NEW"   ,  &pc->IntType,  (union AnyValue *)(&C_FA_CREATE_NEW   ), FALSE );
  VariableDefinePlatformVar( pc, nullptr, "FA_CREATE_ALWAYS",  &pc->IntType,  (union AnyValue *)(&C_FA_CREATE_ALWAYS), FALSE );
  VariableDefinePlatformVar( pc, nullptr, "FA_OPEN_ALWAYS"  ,  &pc->IntType,  (union AnyValue *)(&C_FA_OPEN_ALWAYS  ), FALSE );
  VariableDefinePlatformVar( pc, nullptr, "FA_OPEN_APPEND"  ,  &pc->IntType,  (union AnyValue *)(&C_FA_OPEN_APPEND  ), FALSE );

  IncludeRegister( pc, "oxc_fatfs.h", &oxc_picoc_fatfs_SetupFunc, oxc_picoc_fatfs_Functions, NULL);
}

