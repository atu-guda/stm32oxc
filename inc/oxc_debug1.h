#ifndef _OXC_DEBUG1_H
#define _OXC_DEBUG1_H

#include <oxc_console.h>
// #include <oxc_common1.h>


// number of user one-char vars
inline constexpr unsigned N_USER_VARS = ('z' - 'a' + 1 );
// user vars
extern int user_vars[N_USER_VARS];
#define UVAR(c) (user_vars[(c)-'a'])
inline int& UVAR_a { UVAR('a') };
inline int& UVAR_b { UVAR('b') };
inline int& UVAR_c { UVAR('c') };
inline int& UVAR_d { UVAR('d') };
inline int& UVAR_e { UVAR('e') };
inline int& UVAR_f { UVAR('f') };
inline int& UVAR_g { UVAR('g') };
inline int& UVAR_h { UVAR('h') };
inline int& UVAR_i { UVAR('i') };
inline int& UVAR_j { UVAR('j') };
inline int& UVAR_k { UVAR('k') };
inline int& UVAR_l { UVAR('l') };
inline int& UVAR_m { UVAR('m') };
inline int& UVAR_n { UVAR('n') };
inline int& UVAR_o { UVAR('o') };
inline int& UVAR_p { UVAR('p') };
inline int& UVAR_q { UVAR('q') };
inline int& UVAR_r { UVAR('r') };
inline int& UVAR_s { UVAR('s') };
inline int& UVAR_t { UVAR('t') };
inline int& UVAR_u { UVAR('u') };
inline int& UVAR_v { UVAR('v') };
inline int& UVAR_w { UVAR('w') };
inline int& UVAR_x { UVAR('x') };
inline int& UVAR_y { UVAR('y') };
inline int& UVAR_z { UVAR('z') };

// general buffers
#define GBUF_SZ 256
extern char gbuf_a[GBUF_SZ];
extern char gbuf_b[GBUF_SZ];
extern char* log_buf;
extern unsigned log_buf_size;
extern unsigned log_buf_idx; // gbuf_b is default log place too
void set_log_buf( char *buf, unsigned buf_sz );
void log_add( const char *s );
void log_add_hex( uint32_t v );
void log_add_bin( const char *s, uint16_t len );
void log_reset(void);
void log_print(void);
void print_all_vars();
bool print_given_var( const char *nm, int fmt = 0 );

struct Name2Addr {
  const char *const name;
  void *addr;
};

// helper function : converts some names and numbers to address, fail = -1 (BAD_ADDR)
char* str2addr( const char *str );

void dump8(  const void *addr, unsigned n, bool isAbs = false );
void dump32( const void *addr, unsigned n, bool isAbs = false ); // n in bytes too

bool print_user_var( int idx );

extern bool (*print_var_hook)( const char *nm, int fmt );
extern bool (*set_var_hook)( const char *nm, const char *s );

// arch-dependent function
// fill string s with information about pin config
void gpio_pin_info( GPIO_TypeDef *gi, uint16_t pin, char *s );

void test_delays_misc( int n, uint32_t t_step, int tp );
void test_output_rate( int n, int sl, int do_flush );

extern const char* common_help_string __weak;


#endif

