#ifndef _OXC_CONSOLE_H
#define _OXC_CONSOLE_H

#include <oxc_base.h>

#ifndef NL
  #define NL "\r\n"
#endif


#ifdef __cplusplus
 extern "C" {
#endif

extern int idle_flag;
// define to transfer char to console master
void on_received_char( char c );
typedef int (*CmdFun)(int argc, const char * const * argv );

#define CMDS_NMAX 100
struct CmdInfo
{
  const char *name; //* full command name
  char acr;         //* acronym of command name, or 0
  CmdFun fun;       //* ptr to command
  const char *hint; //* help hint
};
extern const CmdInfo* global_cmds[];


int pr( const char *s ); // redefine in main to current output
int prl( const char *s, int l ); // redefine in main to current output
int pr_d( int d );
int pr_h( uint32_t d );
#define pr_a(a) pr_h( (uint32_t)(a) )
int pr_sd( const char *s, int d );
int pr_sh( const char *s, int d );
#define pr_sdx(x) pr_sd( " " #x "= ", (uint32_t)(x) );
#define pr_shx(x) pr_sh( " " #x "= ", (uint32_t)(x) );


#ifdef __cplusplus
}
#endif

#endif

