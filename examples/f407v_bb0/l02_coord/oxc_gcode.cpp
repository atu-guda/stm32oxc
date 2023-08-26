#include <cctype>
#include <cstring>
#include <oxc_gcode.h>

#define OUT std_out


void GcodeBlock::init()
{
  for( auto &p : fp ) { p =  0; };
  for( auto &p : ip ) { p = uninit_val; };
  str0[0] = str1[0] = '\0';
  err_code = errNone; err_pos = 0;
}

void GcodeBlock::sub_init()
{
  static const unsigned clear_idx[] {
    'X'-'A', 'Y'-'A', 'Z'-'A', 'E'-'A', 'I'-'A', 'J'-'A', 'G'-'A', 'M'-'A', 'T'-'A'
  };
  for( auto i : clear_idx ) {
    fp[i] = 0;
    ip[i] = uninit_val;
  }
}


void GcodeBlock::dump() const
{
  OUT << "# ";
  for( unsigned i=0; i< n_p; ++i ) {
    if( ip[i] == uninit_val ) {
      continue;
    }
    OUT << char( 'A' + i ) << ' ' << fp[i] << "; ";
  }
  OUT << " \"" << str0 << "\" \"" << str1 << "\" " << err_code << ' ' << err_pos << NL;
}




int GcodeBlock::process( const char *s )
{
  if( !s || !ms ) {
    return rcFatal;
  }

  if( s[0] == '%' && s[1] == '\0' ) { // start
    return rcOk;
  }

  auto s0 = s; // to calculate err_pos
  err_pos = 0; err_code = errNone;
  GcodeState st { GcodeState::init }, st0 { st };
  char p_name { ' ' };
  unsigned p_idx { 0 }, n_el { 0 }, n_act { 0 };
  char qs[max_str_sz+1]; qs[0] = '\0'; unsigned qs_len { 0 };
  mach_rc = 0;
  bool to_end { false }; // M117 string and comment to EOL
  bool was_mg { false }; // unhandled G or M was before in string
  const unsigned val_buf_sz { 40 };
  char val_buf[val_buf_sz]; // buffer for digital value
  unsigned vs { 0 };

  for( ; *s; ++s ) { // may be more ++
    char c = *s;
    char *eptr { nullptr };
    xfloat fv { 0 };
    int    iv { 0 };

    switch( st ) {

      case  GcodeState::error :   // -----------------------------------------------------
        continue; // just skip all

      case  GcodeState::init :   // -----------------------------------------------------
        if( isspace(c) ) { continue; }
        if( c == '(' ) { // '[' + store open (param)
          st0 = st; st = GcodeState::comment;
          err_pos = (int)(s-s0); // potential error
          err_code = errComment;
          continue;
        }

        if( c == ';' || c == '*' ) { // EOL comment or CRC (ignored)
          st = GcodeState::comment; to_end = true;
          continue;
        }

        if( c == '"' ) { // free-fly string, for what?
          st0 = st; st =  GcodeState::quoted;
          err_pos = (int)(s-s0); // potential error
          err_code = errString;
          qs[0] = '\0'; qs_len = 0;
          continue;
        }

        if( isalpha( c ) ) {
          p_name = toupper(c);
          p_idx = p_name - 'A';
          if( p_idx >= n_p ) {
            st = GcodeState::error; // or error-catching param
            err_pos = (int)(s-s0);
            err_code = errChar;
            continue;
          }
          // OUT << "## p_name " << p_name << ' ' << p_idx << NL;
          if( ( p_name == 'M' || p_name == 'G' ) && was_mg ) {
            // OUT << "#-- TODO: here to call 1 ms->call_mg" << NL;
            mach_rc = ms->call_mg( this );
            sub_init(); was_mg = false;
            ++n_act; n_el = 0;
          };
          st =  GcodeState::param;
          continue;
        }

        st = GcodeState::error; // unexpected char
        err_pos = (int)(s-s0);
        err_code = errChar;
        continue;

      case  GcodeState::param :   // -----------------------------------------------------
        if( isspace(c) ) { continue; }

        if( c == '"' ) { // only some params?
          st0 = st; st =  GcodeState::quoted;
          fp[p_idx] = 0;  ip[p_idx] = 0;
          err_pos = (int)(s-s0); // potential error
          err_code = errString;
          qs[0] = '\0'; qs_len = 0;
          continue;
        }

        val_buf[0] = '\0';
        vs = strspn( s, "+-.0123456789" );
        if( vs >= val_buf_sz-2 ) {
          err_pos = (int)(s-s0);  err_code = errValue;
          continue;
        }
        memcpy( val_buf, s, vs );
        val_buf[vs] = '\0';
        fv = strtoxf( val_buf, &eptr );
        // OUT << "# dbg_val \"" << val_buf << "\" vs= " << vs << "  fv= " << fv << ' ' << p_name << NL;

        fp[p_idx] = fv;  ip[p_idx] = iv = (int)(fv);
        if( vs > 0 ) {
          s += vs-1;
        } else {
          --s;
        }
        // OUT << "# dbg fv= " << fv << " p_name= " << p_name << " s= \"" << s << '"' << NL;

        if( p_name == 'M' || p_name == 'G' ) {
          was_mg = true;
        }

        if( p_name == 'M' && iv == 117 ) { // special case: string to EOL
          st = GcodeState::quoted; to_end = true;
          qs[0] = '\0'; qs_len = 0;
          ++s; // eat space; TODO: more?
          // OUT << "# dbg Q iv = " << iv << " *s= " << (*s) << NL;
          continue;
        }

        ++n_el;

        p_name = ' '; p_idx = 27; // ????
        st = GcodeState::init;
        break;

      case  GcodeState::quoted : // TODO backslash   // ------------------------------------
        if( c == '"' && !to_end ) {
          st = st0;
          char *os = ( p_name == 'P' ) ? str1 : str0;
          std::strcpy( os, qs );
          err_pos = 0; err_code = errNone;
          qs[0] = '\0'; qs_len = 0;
          continue;
        }
        if( qs_len < max_str_sz-1 ) {
          qs[qs_len++] = c;  qs[qs_len] = '\0';
        }
        continue;

      case  GcodeState::comment :   // -----------------------------------------------------
        if( c == ')' && ! to_end ) {
          st = st0; st = GcodeState::init;  err_pos = 0; err_code = errNone;
        }
        continue;
    };

  }

  int rc { 0 };
  switch( st ) {
    case  GcodeState::error :
      rc = 2;
      break;
    case  GcodeState::init :
      break;
    case  GcodeState::param :
      fp[p_idx] = 0;  ip[p_idx] = 0;
      break;
    case  GcodeState::quoted :
      if( ! to_end ) {
        err_pos = (int)(s-s0);   err_code = errString;
        rc = rcErr;
        break;
      } else {
        char *os = ( p_name == 'P' ) ? str1 : str0;
        std::strcpy( os, qs );
      }
      break;
    case  GcodeState::comment :
      if( ! to_end ) {
        err_pos = (int)(s-s0); err_code = errComment;
        rc = rcErr;
      } else {
      }
      break;
  };

  if( st != GcodeState::error && n_el > 0 ) {
    // OUT << "#-- TODO: here to call 2 ms->call_mg" << NL;
    mach_rc = ms->call_mg( this );
    rc = mach_rc; // TODO: ???
    ++n_act;
    sub_init(); was_mg = false;
  };

  OUT << "# rc= " << rc << NL;

  return rc;
}



int MachStateBase::call_mg( GcodeBlock *cb )
{
  if( !cb ) {
    return GcodeBlock::rcFatal;
  };

  cb->dump(); // debug
  auto is_g = cb->is_set('G');
  auto is_m = cb->is_set('M');
  if( is_g && is_m ) {
    OUT << "#  MS error: M and G" << NL;
    return GcodeBlock::rcErr; // TODO: err_val: both commands
  }

  if( prep_fun ) {
    auto mach_rc = prep_fun( cb, this );
    if( mach_rc >= GcodeBlock::rcErr ) {
      return mach_rc;
    }
  }

  char chfun  = is_m ? 'M' : 'G';
  auto funcs  = is_m ? m_funcs : g_funcs;
  int code    = cb->ipv_or_def( chfun, -1 );

  for( unsigned i=0; i<max_gm_funcs; ++i ) {
    auto [ c, fun ] = funcs[i];
    if( c < 0 ) {
      OUT << "# warn: unsupported " << chfun << ' ' << code << NL;
      return GcodeBlock::rcUnsupp;
    }
    if( c == code ) {
      return fun( cb, this );
    }
  }
  OUT << "# Error! Out of range! " << chfun << ' ' << code << NL;
  return GcodeBlock::rcFatal;
}

