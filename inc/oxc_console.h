#ifndef _OXC_CONSOLE_H
#define _OXC_CONSOLE_H

#include <oxc_base.h>
#include <oxc_io.h>
#include <oxc_miscfun.h>

#ifndef CMDLINE_MAXSZ
#define CMDLINE_MAXSZ 128
#endif

#ifndef MAX_ARGS
#define MAX_ARGS 15
#endif

// future: for substs
#ifndef MAX_ARGLEN
#define MAX_ARGLEN 64
#endif

enum TermKey {
  KEY_NUL =  0,                        /**< ^@ Null character */
  KEY_SOH =  1, KEY_HOME = KEY_SOH,    /**< ^A Start of heading, = HOME */
  KEY_STX =  2, KEY_LEFT = KEY_STX,    /**< ^B Start of text */
  KEY_ETX =  3, KEY_SIGINT = KEY_ETX,  /**< ^C End of text = **INT** */
  KEY_EOT =  4, KEY_DELCH = KEY_EOT,   /**< ^D End of transmission = DEL */
  KEY_ENQ =  5, KEY_END = KEY_ENQ,     /**< ^E Enquiry, goes with ACK = END */
  KEY_ACK =  6, KEY_RIGHT = KEY_ACK,   /**< ^F Acknowledge = RIGHT  */
  KEY_BEL =  7,                        /**< ^G Bell, rings the bell... */
  KEY_BS  =  8, /**< ^H Backspace, works on HP terminals/computers ? */
  KEY_HT  =  9,                        /**< ^I Horizontal tab, move to next tab stop ?? */
  KEY_LF  = 10, /**< ^J Line Feed = LF */
  KEY_VT  = 11, KEY_KILL_END = KEY_VT, /**< ^K Vertical tab = KILL_TO_END */
  KEY_FF  = 12, KEY_REDRAW = KEY_FF,   /**< ^L Form Feed, page eject = REDRAW */
  KEY_CR  = 13, /**< ^M Carriage Return */
  KEY_SO  = 14, KEY_HIST_PREV = KEY_SO, /**< ^N Shift Out */
  KEY_SI  = 15, /**< ^O Shift In, resume defaultn character set ?? */
  KEY_DLE = 16, KEY_HIST_NEXT = KEY_DLE, /**< ^P Data link escape */
  KEY_DC1 = 17, /**< ^Q XON, with XOFF to pause listings; "okay to send". = ?RESUME? */
  KEY_DC2 = 18, /**< ^R Device control 2, block-mode flow control = ?SEARCH? */
  KEY_DC3 = 19, /**< ^S XOFF, with XON is TERM=18 flow control  = ?TTYSTOP? */
  KEY_DC4 = 20, /**< ^T Device control 4 = SWAP(PREW) */
  KEY_NAK = 21, KEY_KILL_BOL = KEY_NAK, /**< ^U Negative acknowledge */
  KEY_SYN = 22, /**< ^V Synchronous idle  = ?LITERAL? */
  KEY_ETB = 23, KEY_KILL_WORD = KEY_ETB, /**< ^W End transmission block */
  KEY_CAN = 24, /**< ^X Cancel line, MPE echoes !!! ???  */
  KEY_EM  = 25, /**< ^Y End of medium, Control-Y interrupt = ?PASTE? */
  KEY_SUB = 26, /**< ^Z Substitute = ??SUSPEND?? */
  KEY_ESC = 27, /**< ^[ Escape, next character is not echoed */
  KEY_FS  = 28, /**< ^\ File separator ?? */
  KEY_GS  = 29, /**< ^] Group separator ?? */
  KEY_RS  = 30, /**< ^^ Record separator, block-mode terminator ?? */
  KEY_US  = 31, /**< ^_ Unit separator ?? */
  KEY_DEL = 127 /**< Delete (not a real control character...) */
};

// man console_codes
void term_cmd1( int n, char c, int fd = 1 );
void term_cmd2( int n1, int n2, char c, int fd = 1 );
void term_move_x( int n, int fd = 1 );
void term_set_x( int n, int fd = 1 );
void term_set_xy( int x, int y, int fd = 1 );
void term_set_scroll_area( int top, int bottom, int fd = 1 );
inline void term_set_00( int fd = 1    ) { prl( "\033[1;1H", 6, fd ); };
inline void term_left1( int fd = 1 )     { prl( "\033[1D", 4, fd ); };
inline void term_right1( int fd = 1 )    { prl( "\033[1C", 4, fd ); };
inline void term_kill_EOL( int fd = 1 )  { prl( "\033[K", 3, fd ); };
inline void term_kill_BOL( int fd = 1 )  { prl( "\033[1K", 4, fd ); };
inline void term_kill_line( int fd = 1 ) { prl( "\033[2K", 4, fd ); };
inline void term_save_cpos( int fd = 1 ) { prl( "\033[s", 3, fd ); };
inline void term_rest_cpos( int fd = 1 ) { prl( "\033[u", 3, fd ); };
inline void term_clear( int fd = 1 )     { prl( "\033[2J", 4, fd ); };

#ifndef CMDLINE_MAX_HANDLERS
#define CMDLINE_MAX_HANDLERS 8
#endif

//* handler can change s, up to CMDLINE_MAXSZ
//  returns: rc = >=0 - line processed, no other actions, return rc
//           rc < 0 - continue processing
typedef int (*CmdlineHandler)( char *s );

extern CmdlineHandler cmdline_handlers[CMDLINE_MAX_HANDLERS];
extern CmdlineHandler cmdline_fallback_handler;


typedef int (*CmdFun)( int argc, const char * const * argv );

#define CMDS_NMAX 100
struct CmdInfo
{
  const char *name; //* full command name
  char acr;         //* acronym of command name, or 0
  CmdFun fun;       //* ptr to command
  const char *hint; //* help hint
};
extern const CmdInfo* global_cmds[];

extern int console_verbose;
volatile extern int on_cmd_handler;


inline int pr_c( char c, int fd = 1 ) { return prl( &c, 1, fd ); }
int pr_d( int d, int fd = 1 );
int pr_h( uint32_t d, int fd = 1 );
#define pr_a(a) pr_h( (uint32_t)(a) )
#define pr_af(a,f) pr_h( (uint32_t)(a), (f) )
int pr_sd( const char *s, int d, int fd = 1 );
int pr_sh( const char *s, int d, int fd = 1 );
#define pr_sdx(x) pr_sd( " " #x "= ", (uint32_t)(x) );
#define pr_sdxf(x,f) pr_sd( " " #x "= ", (uint32_t)(x), (f) );
#define pr_shx(x) pr_sh( " " #x "= ", (uint32_t)(x) );
#define pr_shxf(x,f) pr_sh( " " #x "= ", (uint32_t)(x), (f) );

int cmdline_split( char *cmd, char** argv, int max_args );
int exec_direct( const char *s, int l );
void pr_bitnames( uint32_t b, const BitNames *bn );



#endif

