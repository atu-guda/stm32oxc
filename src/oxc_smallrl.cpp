#include <oxc_console.h>
#include <oxc_smallrl.h>
// #include <oxc_debug1.h>
#include <cstring>

using namespace std;

SMLRL::SmallRL *SMLRL::global_smallrl = nullptr;

SMLRL::SmallRL::SmallRL( ExecFun a_exf, int a_fd /* = 1 */ )
      : exf( a_exf ), fd( a_fd )
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
  pr( NL, fd );
  print_ps1();
  set_good_cpos();
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



// ---------- commands -------------------
int SMLRL::SmallRL::cmd_bs()
{
  if( cpos < 1 ) { return 0; }
  memmove( buf+cpos-1, buf+cpos, epos-cpos+1 );
  --cpos; --epos;
  term_left1( fd );
  term_kill_EOL( fd );
  prl( buf+cpos, epos-cpos+1, fd );
  set_good_cpos();
  return 1;
}

int SMLRL::SmallRL::cmd_delch()
{
  if( epos < 1 || cpos >= epos ) { return 0; }
  memmove( buf+cpos, buf+cpos+1, epos-cpos+1 );
  --epos;
  term_kill_EOL( fd );
  prl( buf+cpos, epos-cpos+1, fd );
  set_good_cpos();
  return 1;
}

int SMLRL::SmallRL::cmd_home()
{
  cpos = 0;
  set_good_cpos();
  return 1;
}

int SMLRL::SmallRL::cmd_left()
{
  if( cpos < 1 ) { return 0; }
  --cpos;
  term_left1( fd );
  return 1;
}

int SMLRL::SmallRL::cmd_right()
{
  if( cpos >=epos ) { return 0; }
  ++cpos;
  term_right1( fd );
  return 1;
}

int SMLRL::SmallRL::cmd_end()
{
  cpos = epos;
  set_good_cpos();
  return 1;
}

int SMLRL::SmallRL::cmd_kill_eol()
{
  epos = cpos; buf[cpos] = 0;
  term_kill_EOL( fd );
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
  term_clear( fd ); term_set_00( fd );
  redraw();
  return 1;
}

int SMLRL::SmallRL::cmd_sigint()
{
  if( !sigf ) { return 0; }
  sigf();
  pr( NL, fd );
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
  prl( buf+cpos, epos-cpos+1, fd );
  ++epos; ++cpos;
  term_set_x( 1+cpos+ps1_vlen, fd  );
  return epos;
}

void SMLRL::SmallRL::redraw() const
{
  term_kill_line( fd );
  term_set_x( 1, fd );
  print_ps1();
  prl( buf, epos, fd );
  set_good_cpos();
}

int SMLRL::SmallRL::history_next()
{
  if( h_cc == h_cur ) { return 0; }
  int nc = history_find_next( h_cc );
  if( nc < 0 ) { return 0; };
  int l = strlen( hist+nc );
  if( l >= bufsz ) { return 0; }
  memmove( buf, hist+nc, l+1 );
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
  memmove( buf, hist+nc, l+1 );
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
    case TermKey::KEY_CR: case TermKey::KEY_LF:
      handle_nl();
      return 1;
    case TermKey::KEY_ESC:
      esc = esc_start;
      return 1;
    case TermKey::KEY_BS: case TermKey::KEY_DEL:
      return cmd_bs();
    case TermKey::KEY_DELCH:
      return cmd_delch();
    case TermKey::KEY_HOME:
      return cmd_home();
    case TermKey::KEY_LEFT:
      return cmd_left();
    case TermKey::KEY_RIGHT:
      return cmd_right();
    case TermKey::KEY_END:
      return cmd_end();
    case TermKey::KEY_KILL_END:
      return cmd_kill_eol();
    case TermKey::KEY_KILL_BOL:
      return cmd_kill_bol();
    case TermKey::KEY_KILL_WORD:
      return cmd_kill_word();
    case TermKey::KEY_REDRAW:
      return cmd_redraw();
    case TermKey::KEY_HIST_NEXT:
      return history_next();
    case TermKey::KEY_HIST_PREV:
      return history_prev();
    case TermKey::KEY_SIGINT:
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
  if( print_cmd ) {
    prl( NL "CMD: \"", 8, fd ); prl( buf, epos, fd ); prl( "\"" NL, 3, fd );
  }
  if( exf && epos > 0 ) {

    if( buf[0] == '.' ) { // DEBUG
      switch( buf[1] ) {
        case 'h':  history_print(); break;
        #ifdef _OXC_DEBUG1_H
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
  memmove( hist+h_cur, buf, epos+1 );
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
  pr( NL "HISTORY:" NL, fd );
  int c = h_cur;

  for( int i=0; i<22; ++i ) {
    int l = strlen( hist + c );
    if( l < 1 ) {   return;   }
    prl( hist+c, l, fd ); pr( NL, fd );

    int nc = history_find_prev( c );
    if( nc < 0 ) { break; }
    c = nc;
  }

}

