#ifndef _OXC_DEBUG1_H
#define _OXC_DEBUG1_H

#include <oxc_console.h>
#include <oxc_common1.h>

#ifdef __cplusplus
 extern "C" {
#endif

// number of user one-char vars
#define N_USER_VARS  ('z' - 'a' + 1 )
// user vars
extern int user_vars[N_USER_VARS];
#define UVAR(c) (user_vars[(c)-'a'])

// general buffers
#define GBUF_SZ 256
extern char gbuf_a[GBUF_SZ];
extern char gbuf_b[GBUF_SZ];
extern int log_buf_idx; // gbuf_b is log place too
void log_add( const char *s );
void log_add_bin( const char *s, uint16_t len );
void log_reset(void);
void log_print(void);

// helper function : converts some names and numbers to address, fail = -1 (BAD_ADDR)
char* str2addr( const char *str );

void dump8( const void *addr, int n, bool isAbs = false );

void print_user_var( int idx );

// arch-dependent function
// fill string s with information about pin config
void gpio_pin_info( GPIO_TypeDef *gi, uint16_t pin, char *s );

extern const char* common_help_string __weak;

// common commands:
int cmd_info( int argc, const char * const * argv );
extern CmdInfo CMDINFO_INFO;
int cmd_echo( int argc, const char * const * argv );
extern CmdInfo CMDINFO_ECHO;
int cmd_help( int argc, const char * const * argv );
extern CmdInfo CMDINFO_HELP;
int cmd_dump( int argc, const char * const * argv );
extern CmdInfo CMDINFO_DUMP;
int cmd_fill( int argc, const char * const * argv );
extern CmdInfo CMDINFO_FILL;
int cmd_pvar( int argc, const char * const * argv );
extern CmdInfo CMDINFO_PVAR;
int cmd_svar( int argc, const char * const * argv );
extern CmdInfo CMDINFO_SVAR;
int cmd_die( int argc, const char * const * argv );
extern CmdInfo CMDINFO_DIE;
int cmd_reboot( int argc, const char * const * argv );
extern CmdInfo CMDINFO_REBOOT;
int cmd_log_print( int argc, const char * const * argv );
extern CmdInfo CMDINFO_LOG_PRINT;
int cmd_log_reset( int argc, const char * const * argv );
extern CmdInfo CMDINFO_LOG_RESET;
int cmd_pin_info( int argc, const char * const * argv );
extern CmdInfo CMDINFO_PIN_INFO;
int cmd_set_leds_step( int argc, const char * const * argv );
extern CmdInfo CMDINFO_LSTEP;

#define DEBUG_CMDS \
  &CMDINFO_HELP, \
  &CMDINFO_INFO, \
  &CMDINFO_DUMP, \
  &CMDINFO_FILL, \
  &CMDINFO_ECHO, \
  &CMDINFO_REBOOT, \
  &CMDINFO_DIE, \
  &CMDINFO_LOG_PRINT, \
  &CMDINFO_LOG_RESET, \
  &CMDINFO_PVAR, \
  &CMDINFO_SVAR, \
  &CMDINFO_PIN_INFO, \
  &CMDINFO_LSTEP \

#ifdef __cplusplus
}
#endif

#endif

