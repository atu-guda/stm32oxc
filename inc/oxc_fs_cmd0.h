#ifndef _OXC_FS_CMD0_H
#define _OXC_FS_CMD0_H

#include <oxc_console.h>
#include <oxc_debug1.h>

const int fspath_sz = 32;
extern char fspath[fspath_sz];

int cmd_mount( int argc, const char * const * argv );
extern CmdInfo CMDINFO_MOUNT;
int cmd_umount( int argc, const char * const * argv );
extern CmdInfo CMDINFO_UMOUNT;
int cmd_fsinfo( int argc, const char * const * argv );
extern CmdInfo CMDINFO_FSINFO;
int cmd_ls( int argc, const char * const * argv );
extern CmdInfo CMDINFO_LS;
int cmd_cat( int argc, const char * const * argv );
extern CmdInfo CMDINFO_CAT;
int cmd_appstr( int argc, const char * const * argv );
extern CmdInfo CMDINFO_APPSTR;
int cmd_wblocks( int argc, const char * const * argv );
extern CmdInfo CMDINFO_WBLOCKS;
int cmd_rm( int argc, const char * const * argv );
extern CmdInfo CMDINFO_RM;

#define FS_CMDS0 \
  &CMDINFO_MOUNT, \
  &CMDINFO_UMOUNT, \
  &CMDINFO_FSINFO, \
  &CMDINFO_LS, \
  &CMDINFO_CAT, \
  &CMDINFO_APPSTR, \
  &CMDINFO_WBLOCKS, \
  &CMDINFO_RM


#endif

