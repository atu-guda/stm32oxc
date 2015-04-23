#include <oxc_console.h>
#include <oxc_smallrl.h>
// #include <oxc_debug1.h>
#include <cstring>

SMLRL::SmallRL *SMLRL::global_smallrl = nullptr;

void SMLRL::u2_3dig( unsigned n, char *s )
{
  if( !s ) { return; }
  s[2] = '0' + n % 10; n /= 10;
  s[1] = '0' + n % 10; n /= 10;
  s[0] = '0' + n % 10;
}

int SMLRL::cmdline_split( char *cmd, char** argv, int max_args )
{
  if( !cmd || !argv || ! *cmd ) { return 0; }
  int nc = 0;
  bool was_bs = false;
  char was_quo = 0;

  int l = strlen( cmd ); // need, as we adds many nulls
  int j = 0;

  for( int i=0; i<=l; ++i ) { // ??? <= ???
    char c = cmd[i];
    // cerr << "c: '" << c << "' was_bs: " << was_bs << " i= " << i << " j= " << j << endl;

    if( was_bs ) {
      was_bs = false;
      char c1 = c;
      switch( c ) {
        case 'n': c1 = '\n'; break;
        case 'r': c1 = '\r'; break;
        case 't': c1 = '\t'; break;
        case 'e': c1 = '\033'; break;
      }
      cmd[j++] = c1;
      continue;
    }

    if( c == '\\' && was_quo != '\'' ) {
      was_bs = true;
      continue;
    }

    if( was_quo ) {
      if( c == was_quo ) { was_quo = 0; continue; }
      cmd[j++] = c;
      continue;
    }

    if( c == '\'' || c == '"' ) {
      was_quo = c; continue;
    }

    if( c == ' ' ) {
      cmd[j++] = '\0'; continue;
    }
    cmd[j++] = c;
  }
  for( int i=j; i<=l; ++i ) { cmd[i] = '\0'; } // <= !!!

  if( was_quo ) { return 0; }


  bool was_nul = true; // start assumed as nul
  char c;

  for( int i=0; i<l; ++i ) {
    c = cmd[i];
    bool is_nul = ( c == '\0' );

    if( was_nul && ! is_nul ) {
      argv[nc++] = cmd+i;
      if( nc >= (max_args-1) ) { break; } // (-1): place for last null
    }

    was_nul = is_nul;
  };


  argv[nc] = nullptr;
  return nc;
}

int SMLRL::exec_direct( const char *s, int l )
{
  char ss[l+1];
  memcpy( ss, s, l+1 );
  char *argv[MAX_ARGS];
  int argc = cmdline_split( ss, argv, MAX_ARGS );
  // DEBUG
  // dump8( s,  l+1 );
  // dump8( ss, l+1 );

  if( argc < 1 ) { return 1; }

  CmdFun f = 0;
  const char *nm = "???";

  for( int i=0; global_cmds[i] && i<CMDS_NMAX; ++i ) {
    if( global_cmds[i]->name == 0 ) {
      break;
    }
    if( argv[0][1] == '\0'  &&  argv[0][0] == global_cmds[i]->acr ) {
      f = global_cmds[i]->fun;
      nm = global_cmds[i]->name;
      break;
    }
    if( strcmp( global_cmds[i]->name, argv[0])  == 0 ) {
      f = global_cmds[i]->fun;
      nm = global_cmds[i]->name;
      break;
    }
  }

  if( f != 0 ) {
      int rc = 0;
      pr( NL "=== CMD: \"" ); pr( nm ); pr( "\"" NL );
      delay_ms( 50 );
      rc = f( argc, argv );
      pr_sdx( rc );
  } else {
    pr( "ERR:  Unknown command \"" );  pr( argv[0] );   pr( "\"" NL );
  }

  return 0;
}

// ----------------------- SmallRL ------------------------------

SMLRL::SmallRL::SmallRL( PrintFun a_prf, ExecFun a_exf )
      : prf( a_prf ), exf( a_exf )
{
  ps1[0] = '#'; ps1[1] = ' '; ps1[2] = 0;
  reset();
};

void SMLRL::SmallRL::reset()
{
  buf[0] = 0; cpos = epos; esc = esc_no;
  memset( hist, '\0', histsz );
  h_cur = h_cc = h_end = 0;
}

void SMLRL::SmallRL::re_ps() const
{
  puts( NL );
  print_ps1();
  term_good_cpos();
}

void SMLRL::SmallRL::set_ps1( const char *p, int a_vlen )
{
  if( !p ) { return; }
  int l = strlen( p );
  if( l >= ps1sz ) { return; }
  if( a_vlen == 0 ) {
    a_vlen = l;
  }
  strcpy( ps1, p );
  ps1_len = l; ps1_vlen = a_vlen;
}

void SMLRL::SmallRL:: puts( const char *s ) const
{
  if( !s || !prf ) { return; }
  int l = strlen( s );
  if( l < 1 ) { return; }
  prf( s, l );
}

void SMLRL::SmallRL::term_cmd1( int n, char c ) const
{
  char b[8];
  b[0] = '\033'; // ESC
  b[1] = '[';
  u2_3dig( n, b+2 ); // 2,3,4
  b[5] = c; b[6] = 0;
  prf( b, 6 );
}

void SMLRL::SmallRL::term_move_x( int n ) const
{
  if( n < 0 ) {
    term_cmd1( -n, 'D' );
  } else {
    term_cmd1(  n, 'C' );
  }
}

void SMLRL::SmallRL::term_set_x( int n ) const
{
  term_cmd1(  n, 'G' );
}

// ---------- commands -------------------
int SMLRL::SmallRL::cmd_bs()
{
  if( cpos < 1 ) { return 0; }
  memmove( buf+cpos-1, buf+cpos, epos-cpos+1 );
  --cpos; --epos;
  term_left1();
  term_kill_EOL();
  prf( buf+cpos, epos-cpos+1 );
  term_good_cpos();
  return 1;
}

int SMLRL::SmallRL::cmd_delch()
{
  if( epos < 1 || cpos >= epos ) { return 0; }
  memmove( buf+cpos, buf+cpos+1, epos-cpos+1 );
  --epos;
  term_kill_EOL();
  prf( buf+cpos, epos-cpos+1 );
  term_good_cpos();
  return 1;
}

int SMLRL::SmallRL::cmd_home()
{
  cpos = 0;
  term_good_cpos();
  return 1;
}

int SMLRL::SmallRL::cmd_left()
{
  if( cpos < 1 ) { return 0; }
  --cpos;
  term_left1();
  return 1;
}

int SMLRL::SmallRL::cmd_right()
{
  if( cpos >=epos ) { return 0; }
  ++cpos;
  term_right1();
  return 1;
}

int SMLRL::SmallRL::cmd_end()
{
  cpos = epos;
  term_good_cpos();
  return 1;
}

int SMLRL::SmallRL::cmd_kill_eol()
{
  epos = cpos; buf[cpos] = 0;
  term_kill_EOL();
  return 1;
}

int SMLRL::SmallRL::cmd_kill_bol()
{
  if( cpos < 1 ) { return 0; }
  memmove( buf, buf+cpos, epos-cpos+1 );
  cpos = 0; epos -= cpos;
  redraw();
  return 1;
}

int SMLRL::SmallRL::cmd_kill_word()
{
  if( cpos < 1 ) { return 0; }
  int pe;
  for( pe=cpos-1; pe>0 && buf[pe] == ' '; --pe ) /* eat ws */ ;
  for( ; pe>0 && buf[pe] != ' '; --pe ) /* eat non-ws */ ;

  memmove( buf+pe, buf+cpos, epos-cpos+1 );
  epos -= cpos - pe; cpos = pe;
  redraw();
  return 1;
}


int SMLRL::SmallRL::cmd_redraw()
{
  term_clear(); term_set_00();
  redraw();
  return 1;
}

int SMLRL::SmallRL::cmd_sigint()
{
  if( !sigf ) { return 0; }
  sigf();
  puts( NL );
  redraw();
  return 1;
}

int SMLRL::SmallRL::cmd_addchar( char c )
{
  if( epos >= bufsz-1 ) {
    return 0;
  }
  memmove( buf+cpos+1, buf+cpos, epos-cpos+1 );
  buf[cpos] = c;
  prf( buf+cpos, epos-cpos+1 );
  ++epos; ++cpos;
  term_set_x( 1+cpos+ps1_vlen );
  return epos;
}

void SMLRL::SmallRL::redraw() const
{
  term_kill_line();
  term_set_x( 1 );
  print_ps1();
  prf( buf, epos );
  term_good_cpos();
}

int SMLRL::SmallRL::history_next()
{
  if( h_cc == h_cur ) { return 0; }
  int nc = history_find_next( h_cc );
  if( nc < 0 ) { return 0; };
  int l = strlen( hist+nc );
  if( l >= bufsz ) { return 0; }
  memcpy( buf, hist+nc, l+1 );
  epos = cpos = l;
  h_cc = nc;
  redraw();
  return 1;
}

int SMLRL::SmallRL::history_prev()
{
  int nc;
  if( h_cc == h_cur && epos > 0 ) {
    history_add_cur();
    nc = history_find_prev( h_cc );
  } else {
    nc = h_cc;
  }

  int l = strlen( hist+nc );
  if( l >= bufsz ) { return 0; }
  memcpy( buf, hist+nc, l+1 );
  epos = cpos = l;
  h_cc = nc;
  redraw();

  nc = history_find_prev( nc );
  if( nc >= 0 ) { h_cc = nc; };
  return 1;
}

int SMLRL::SmallRL::addChar( char c )
{
  if( esc != esc_no ) {
    return handle_esc( c );
  }

  switch( c ) {
    case KEY_CR: case KEY_LF:
      handle_nl();
      return 1;
    case KEY_ESC:
      esc = esc_start;
      return 1;
    case KEY_BS: case KEY_DEL:
      return cmd_bs();
    case KEY_DELCH:
      return cmd_delch();
    case KEY_HOME:
      return cmd_home();
    case KEY_LEFT:
      return cmd_left();
    case KEY_RIGHT:
      return cmd_right();
    case KEY_END:
      return cmd_end();
    case KEY_KILL_END:
      return cmd_kill_eol();
    case KEY_KILL_BOL:
      return cmd_kill_bol();
    case KEY_KILL_WORD:
      return cmd_kill_word();
    case KEY_REDRAW:
      return cmd_redraw();
    case KEY_HIST_NEXT:
      return history_next();
    case KEY_HIST_PREV:
      return history_prev();
    case KEY_SIGINT:
      return cmd_sigint();
    default:
      break;
  }

  if( (unsigned char)(c) < ' ' ) { // ignore unused control codes
    return 0;
  }

  return cmd_addchar( c );
}


void SMLRL::SmallRL::handle_nl()
{
  if( exf && epos > 0 ) {

    if( buf[0] == '.' ) { // DEBUG
      switch( buf[1] ) {
        case 'h':  history_print(); break;
        #ifdef _OX_DEBUG1_H
          case 'd':  dump8( hist, 16 ); break;
        #endif
        case 'r':  reset(); break;
        default: break;
      }
    } else { // ordinary command
      history_add_cur();
      // dump8( buf, epos );
      exf( buf, epos );
    }

  }
  cpos = epos = 0; buf[0] = 0;
  re_ps();
}


int SMLRL::SmallRL::handle_esc( char c )
{
  if( esc == esc_no ) { return 0; }

  if( esc == esc_start ) {
    switch( c ) {
      case '[' : esc = esc_bracket; return 1;
      case '~' : esc = esc_tild;    return 1;
      // TODO: more
      default:   esc = esc_no;      return 0;
    }
  }

  if( esc == esc_bracket ) {
    esc = esc_no;
    switch( c ) {
      case 'D' : cmd_left(); return 1;
      case 'C' : cmd_right(); return 1;
      case 'H' : cmd_home(); return 1;
      case 'F' : cmd_end(); return 1;
      case 'A' : history_prev(); return 1;
      case 'B' : history_next(); return 1;
      case '2' : /*cmd_ins()*/;  esc = esc_tild; return 1;
      case '3' : cmd_delch();    esc = esc_tild; return 1;
      case '5' : /*cmd_pgup()*/; esc = esc_tild; return 1;
      case '6' : /*cmd_pgdn()*/; esc = esc_tild; return 1;
      // TODO: more
      default:  return 0;
    }
  }

  if( esc == esc_tild ) {
    esc = esc_no;
    return 0;
  }

  esc = esc_no; // fallback
  return 0;
}


int SMLRL::SmallRL::history_add_cur()
{
  if( epos >= histsz-1  || buf[0] == ' ' ) { return 0; }

  if( h_end + epos >= histsz ) { // roll
    h_end = 0;
  }

  h_cc = h_cur = h_end;
  memcpy( hist+h_cur, buf, epos+1 );
  h_end += epos+1;
  if( h_end >= histsz ) { h_end = 0; }
  for( int i=h_end; i<histsz && hist[i]!= '\0'; ++i ) { // rm old cmd corpse
    hist[i] = '\0';
  }
  return h_end;

}

int  SMLRL::SmallRL::history_find_prev( int cp ) const
{
  int nc = cp-1, ni, sta = 0;
  for( ni=0; ni<histsz && nc != h_cur; ++ni ) {
    nc += (histsz-1); nc %= histsz;

    if( sta == 0 ) {
      if( hist[nc] != '\0' ) { sta = 1; }
      continue;
    }
    if( sta == 1 ) {
      if( hist[nc] == '\0' ) { sta = 2; break; }
      continue;
    }
  }
  if( sta != 2 ) { return -1; }
  ++nc; nc %= histsz;
  return nc;
}

int  SMLRL::SmallRL::history_find_next( int cp ) const
{
  int nc = cp, ni, sta = 0;
  for( ni=0; ni<histsz && nc != h_cur; ++ni ) {
    ++nc; nc %= histsz;

    if( sta == 0 ) {
      if( hist[nc] == '\0' ) { sta = 1; }
      continue;
    }
    if( sta == 1 ) {
      if( hist[nc] != '\0' ) { sta = 2; break; }
      continue;
    }
  }
  if( sta != 2 ) { return -1; }
  return nc;
}

void SMLRL::SmallRL::history_print() const
{
  puts( NL "HISTORY:" NL );
  int c = h_cur;

  for( int i=0; i<22; ++i ) {
    int l = strlen( hist + c );
    if( l < 1 ) {   return;   }
    prf( hist+c, l ); puts( NL );

    int nc = history_find_prev( c );
    if( nc < 0 ) { break; }
    c = nc;
  }

}

