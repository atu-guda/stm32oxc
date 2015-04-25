#ifndef _OXC_SMALLRL_H
#define _OXC_SMALLRL_H

#include <oxc_base.h>

#ifndef SMLRL_BUFSZ
#define SMLRL_BUFSZ 128
#endif
#ifndef SMLRL_HISTSZ
#define SMLRL_HISTSZ (SMLRL_BUFSZ*3)
#endif

#ifndef NL
#define NL "\r\n"
#endif

#ifndef MAX_ARGS
#define MAX_ARGS 9
#endif

// man console_codes

namespace SMLRL {

  void u2_3dig( unsigned n, char *s ); // 3 chars w/o 0
  int cmdline_split( char *cmd, char** argv, int max_args ); // modifies cmd!
  int exec_direct( const char *s, int l );

  enum keys {
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

  enum EscState {
    esc_no = 0, esc_start, esc_bracket, esc_tild
  };

  using PrintFun = int (*)( const char *s, int len );
  using ExecFun  = int (*)( const char *s, int len );
  using SigFun   = void(*)();


  class SmallRL {
    public:
     SmallRL( PrintFun a_prf, ExecFun a_exf ); // how to print, what to exec
     int addChar( char c );                    // input stream
     void setSigFun( SigFun a_sigf ) { sigf = a_sigf; } // on Ctrl-C
     void reset();
     void re_ps() const;
     static const constexpr int bufsz = SMLRL_BUFSZ;
     static const constexpr int histsz = SMLRL_HISTSZ;
     static const constexpr int ps1sz = 32;
     const char *get() const { return buf; }
     void print_ps1() const { prf( ps1, ps1_len ); }
     void redraw() const;
     void set_ps1( const char *p, int a_vlen = 0 );
     void puts( const char *s ) const;
     void set_print_cmd( bool pr ) { print_cmd = pr; }

     void term_cmd1( int n, char c ) const;
     void term_move_x( int n ) const;
     void term_set_x( int n ) const;
     void term_set_00() const    { prf( "\033[1;1H", 6 ); };
     void term_left1() const     { prf( "\033[1D", 4 ); };
     void term_right1() const    { prf( "\033[1C", 4 ); };
     void term_kill_EOL() const  { prf( "\033[K", 3 ); };
     void term_kill_BOL() const  { prf( "\033[1K", 4 ); };
     void term_kill_line() const { prf( "\033[2K", 4 ); };
     void term_save_cpos() const { prf( "\033[s", 3 ); };
     void term_rest_cpos() const { prf( "\033[u", 3 ); };
     void term_good_cpos() const { term_set_x( cpos+1 + ps1_vlen ); }
     void term_clear() const     { prf( "\033[2J", 4 ); };

     int cmd_bs();
     int cmd_delch();
     int cmd_home();
     int cmd_left();
     int cmd_right();
     int cmd_end();
     int cmd_kill_eol();
     int cmd_kill_bol();
     int cmd_kill_word();
     int cmd_redraw();
     int cmd_addchar( char c );
     int cmd_sigint();

     int history_next();
     int history_prev();
    private:
     char buf[bufsz];
     char ps1[ps1sz];
     char hist[histsz];
     int epos = 0; // end posision
     int cpos = 0; // cursor posision
     int h_cur = 0; // current history pos
     int h_cc = 0; // current-current
     int h_end = 0; // end history pos (start of new)
     EscState esc = esc_no;  // escape state
     int ps1_len = 2, ps1_vlen = 2; // real and visual ps1 sizes
     bool print_cmd = false;
     PrintFun prf;
     ExecFun exf;
     SigFun  sigf = nullptr;

     void handle_nl();
     int  handle_esc( char c );

     int  history_add_cur();
     int  history_find_prev( int cp ) const;
     int  history_find_next( int cp ) const;
     void history_print() const;
  };

  extern SmallRL *global_smallrl;

}; // namespace SMLRL

#endif

