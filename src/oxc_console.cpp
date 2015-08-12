#include <string.h>
#include <oxc_console.h>
#include <oxc_devio.h>

void term_cmd1( int n, char c, int fd )
{
  char b[8];
  b[0] = '\033'; // ESC
  b[1] = '[';
  u2_3dig( n, b+2 ); // 2,3,4
  b[5] = c; b[6] = 0;
  prl( b, 6, fd );
}

void term_cmd2( int n1, int n2, char c, int fd )
{
  char b[12];
  b[0] = '\033'; // ESC
  b[1] = '[';
  u2_3dig( n1, b+2 ); // 2,3,4
  b[5] = ';';
  u2_3dig( n2, b+6 ); // 6,7,8
  b[9] = c; b[10] = 0;
  prl( b, 10, fd );
}

void term_move_x( int n, int fd )
{
  if( n < 0 ) {
    term_cmd1( -n, 'D', fd );
  } else {
    term_cmd1(  n, 'C', fd );
  }
}

void term_set_x( int n, int fd )
{
  term_cmd1(  n, 'G', fd );
}

void term_set_xy( int x, int y, int fd )
{
  term_cmd2( y, x, 'H', fd );
}

void term_set_scroll_area( int top, int bottom, int fd )
{
  if( top < 0 || bottom < 0 ) {
    prl( "\033[r", 3, fd );
  } else {
    term_cmd2( top, bottom, 'r', fd );
  }
}


int pr_d( int d, int fd )
{
  char b[INT_STR_SZ_DEC];
  return pr( i2dec( d, b ), fd );
}

int pr_h( uint32_t d, int fd )
{
  char b[INT_STR_SZ_HEX];
  return pr( word2hex( d, b ), fd );
}

int pr_sd( const char *s, int d, int fd )
{
  pr( s, fd );
  pr_d( d, fd );
  return pr( NL );
}

int pr_sh( const char *s, int d, int fd )
{
  pr( s, fd );
  pr_h( d, fd );
  return pr( NL );
}

int cmdline_split( char *cmd, char** argv, int max_args )
{
  if( !cmd || !argv || ! *cmd ) { return 0; }
  int nc = 0;
  bool was_bs = false;
  char was_quo = 0;

  int l = strlen( cmd ); // need, as we adds many nulls
  if( l < 1 || l > CMDLINE_MAXSZ-4 ) {
    return 0;
  }
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

int exec_direct( const char *s, int l )
{
  if( l<0 || l >= CMDLINE_MAXSZ ) {
    return 0;
  }
  // dump8( s,  l+1 );
  char ss[l+1];
  // static char ss[SMLRL_BUFSZ+2]; // TODO: lock!
  // memcpy( ss, s, l+1 );
  memmove( ss, s, l+1 );
  // dump8( ss, l+1 );

  char *argv[MAX_ARGS];
  int argc = cmdline_split( ss, argv, MAX_ARGS );
  // DEBUG
  // dump8( ss, l+1 );

  if( argc < 1 ) { return 1; }

  CmdFun f = 0;
  // const char *nm = "???";

  for( int i=0; global_cmds[i] && i<CMDS_NMAX; ++i ) {
    if( global_cmds[i]->name == 0 ) {
      break;
    }
    if( argv[0][1] == '\0'  &&  argv[0][0] == global_cmds[i]->acr ) {
      f = global_cmds[i]->fun;
      // nm = global_cmds[i]->name;
      break;
    }
    if( strcmp( global_cmds[i]->name, argv[0])  == 0 ) {
      f = global_cmds[i]->fun;
      // nm = global_cmds[i]->name;
      break;
    }
  }

  if( f != 0 ) {
      int rc = 0;
      // pr( NL "=== CMD: \"", fd ); pr( nm, fd ); pr( "\"" NL, fd );
      // delay_ms( 50 );
      rc = f( argc, argv );
      pr_sdx( rc );
  } else {
    pr( "ERR:  Unknown command \"" );  pr( argv[0] );   pr( "\"" NL );
  }

  return 0;
}

// ------------------------ BarHText ----------------------------------

BarHText::BarHText( int x, int y, int w, int v_min, int v_max, int a_fd  )
  : x0( x ), y0( y ), w0( w-2 ), vmin( v_min ), vmax( v_max ), fd( a_fd ),
    i0( ( 0 - vmin ) * w0 / ( vmax - vmin ) )
{
  c_zero_e = c_zero;
  if( i0 < 0 )   { i0 = 0;    c_zero_e = '<'; };
  if( i0 >= w0 ) { i0 = w0-1; c_zero_e = '>'; };
}

void BarHText::draw( int v )
{
  if( w0 < 3 ) { return; };
  char lbra = '[', rbra = ']';
  if( v < vmin ) { v = vmin; lbra = '$'; };
  if( v > vmax ) { v = vmax; rbra = '$'; };

  int iv = ( v - vmin ) * w0 / ( vmax - vmin );
  char zero_char = c_zero_e;

  int lb = i0, rb = iv;
  bool left_zero = true;
  if( lb > rb ) { int t = lb; lb = rb; rb = t; left_zero = false; }

  if( useCodes ) {
    term_save_cpos( fd );
    term_set_xy( x0, y0, fd );
  }


  pr_c( lbra, fd );
  int i; // not in loop
  for( i=0; i<lb-1; ++i ) {
    pr_c( c_space, fd );
  }
  pr_c( ( left_zero && drawZero ) ? zero_char : c_space ); ++i;
  for( /* */; i<rb-2; ++i ) {
    pr_c( c_bar, fd );
  }
  pr_c( ( !left_zero && drawZero ) ? zero_char : c_bar ); ++i;
  for( /* */; i<w0; ++i ) {
    pr_c( c_space, fd );
  }
  pr_c( rbra, fd );

  if( useCodes ) {
    term_rest_cpos( fd );
  }
}

