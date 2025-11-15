#ifndef _OXC_FS_CMD0_H
#define _OXC_FS_CMD0_H

const int fspath_sz = 32;
extern char fspath[fspath_sz];

int cmd_mount( int argc, const char * const * argv );
int cmd_umount( int argc, const char * const * argv );
int cmd_fsinfo( int argc, const char * const * argv );
int cmd_ls( int argc, const char * const * argv );
int cmd_cat( int argc, const char * const * argv );
int cmd_appstr( int argc, const char * const * argv );
int cmd_wblocks( int argc, const char * const * argv );
int cmd_rm( int argc, const char * const * argv );


//

#endif

