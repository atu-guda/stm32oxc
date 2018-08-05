#ifndef _OXC_SMALLRL_H
#define _OXC_SMALLRL_H

#include <oxc_console.h>


#ifndef SMLRL_HISTSZ
#define SMLRL_HISTSZ (CMDLINE_MAXSZ*3)
#endif


namespace SMLRL {

  enum EscState {
    esc_no = 0, esc_start, esc_bracket, esc_tild
  };

  using ExecFun  = int (*)( const char *s, int len );
  using PostExecFun  = int (*)( int rc );
  using SigFun   = void(*)();


  class SmallRL {
    public:
     SmallRL( ExecFun a_exf, int a_fd = 1 ); // how to print, what to exec
     SmallRL( const SmallRL &r ) = delete;
     int addChar( char c );                    // input stream
     void setSigFun( SigFun a_sigf ) { sigf = a_sigf; } // on Ctrl-C
     void setPostExecFun( PostExecFun a_post_exf ) { post_exf = a_post_exf; }
     void reset();
     void re_ps() const;
     static const constexpr int bufsz = CMDLINE_MAXSZ;
     static const constexpr int histsz = SMLRL_HISTSZ;
     static const constexpr int ps1sz = 32;
     static const constexpr int max_hist_out_lines = 22;
     const char *get() const { return buf; }
     void print_ps1() const { prl( ps1, ps1_len, fd ); }
     void redraw() const;
     void set_ps1( const char *p, int a_vlen = 0 );
     void set_print_cmd( bool pr ) { print_cmd = pr; }
     void set_good_cpos() const { term_set_x( cpos+1 + ps1_vlen ); }

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
    protected:
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
     ExecFun exf;
     PostExecFun post_exf = nullptr;
     int fd;
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

#define STD_POST_EXEC int standart_post_exec( int rc ) {   dev_console.reset_in();   return rc; }

#endif

