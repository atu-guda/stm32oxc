// #include <iostream>
#include <algorithm>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <cmath>

#include <oxc_auto.h>
#include "meas0.h"
#include "tcalclang.h"

using namespace std;
using namespace tcalclang;


const char* tcalclang::dtype2name( DataType dt )
{
  switch( dt ) {
    case DataType::t_none    : return "none";
    case DataType::t_int     : return "int";
    case DataType::t_float   : return "float";
    case DataType::t_int_i   : return "int_i";
    case DataType::t_float_i : return "float_i";
    case DataType::t_bad     : return "bad";
    default: break;
  }
  return "?";
}


const CmdOpInfo tcalclang::cmdOpInfos[] = {
  { CmdOp::exit  ,  0 , "exit"  } ,
  { CmdOp::err   ,  0 , "abort" } ,
  { CmdOp::nop   ,  0 , "nop"   } ,
  { CmdOp::add   ,  1 , "+"     } ,
  { CmdOp::sub   ,  1 , "-"     } ,
  { CmdOp::mul   ,  1 , "*"     } ,
  { CmdOp::div   ,  1 , "/"     } ,
  { CmdOp::mod   ,  1 , "%"     } ,
  { CmdOp::stor  ,  1 , "S"     } ,
  { CmdOp::load  ,  1 , "L"     } ,
  { CmdOp::loady ,  1 , "LY"    } ,
  { CmdOp::loadz ,  1 , "LZ"    } ,
  { CmdOp::loadn ,  1 , "LN"    } ,
  { CmdOp::fun0  ,  0 , ""      } ,
  { CmdOp::fun1  , -1 , ""      } ,
  { CmdOp::fun2  , -2 , ""      } ,
  { CmdOp::fun3  , -3 , ""      } ,
  { CmdOp::pi    ,  0 , "M_PI"  } ,
};

const CmdOpInfo* tcalclang::findCmdOp( const char *s, const char** eptr )
{
  char nm[max_func_name_length];
  if( eptr ) {
    *eptr = s;
  }
  skip_ws( s );
  unsigned nm_len = 0;
  while( *s != ' ' && *s != '\0' && *s != ';' && nm_len < max_func_name_length-1 ) {
    nm[nm_len++] = *s++;
  }
  nm[nm_len] = '\0';
  skip_ws( s );

  for( const auto &oi : cmdOpInfos ) {
    if( strcmp( oi.nm, nm ) == 0 ) {
      if( eptr ) {
        *eptr = s;
      }
      return &oi;
    }
  }

  return nullptr;
}

const FuncInfo tcalclang::funcInfos[] = {
  // {  "random" },
  { "sin"      , (void*)sinf      , 1 } ,
  { "cos"      , (void*)cosf      , 1 } ,
  { "tan"      , (void*)tanf      , 1 } ,
  { "asin"     , (void*)asinf     , 1 } ,
  { "acos"     , (void*)acosf     , 1 } ,
  { "atan"     , (void*)atanf     , 1 } ,
  { "sinh"     , (void*)sinhf     , 1 } ,
  { "cosh"     , (void*)coshf     , 1 } ,
  { "tanh"     , (void*)tanhf     , 1 } ,
  { "asinh"    , (void*)asinhf    , 1 } ,
  { "acosh"    , (void*)acoshf    , 1 } ,
  { "atanh"    , (void*)atanhf    , 1 } ,
  { "exp"      , (void*)expf      , 1 } ,
  { "exp2"     , (void*)exp2f     , 1 } ,
  { "expm1"    , (void*)expm1f    , 1 } ,
  { "sqrt"     , (void*)sqrtf     , 1 } , // may be sqrt0f?
  { "cbrt"     , (void*)cbrtf     , 1 } ,
  { "fabs"     , (void*)fabsf     , 1 } ,
  { "fdim"     , (void*)fdimf     , 1 } ,
  { "round"    , (void*)roundf    , 1 } ,
  { "ceil"     , (void*)ceilf     , 1 } ,
  { "floor"    , (void*)floorf    , 1 } ,
  { "log"      , (void*)logf      , 1 } ,
  { "log1p"    , (void*)log1pf    , 1 } ,
  { "log2"     , (void*)log2f     , 1 } ,
  { "log10"    , (void*)log10f    , 1 } ,
  { "atan2"    , (void*)atan2f    , 2 } ,
  { "hypot"    , (void*)hypotf    , 2 } ,
  { "fmax"     , (void*)fmaxf     , 2 } ,
  { "fmin"     , (void*)fminf     , 2 } ,
  { "pow"      , (void*)powf      , 2 } ,
  { "copysign" , (void*)copysignf , 2 } ,

};

const FuncInfo* tcalclang::findFunc( const char *s, const char** eptr )
{
  char nm[max_func_name_length];
  if( eptr ) {
    *eptr = s;
  }
  skip_ws( s );
  unsigned nm_len = 0;
  while( isNameChar( *s ) && nm_len < max_func_name_length-1 ) {
    nm[nm_len++] = *s++;
  }
  nm[nm_len] = '\0';
  skip_ws( s );

  for( const auto &fi : funcInfos ) {
    if( strcmp( fi.nm, nm ) == 0 ) {
      if( eptr ) {
        *eptr = s;
      }
      return &fi;
    }
  }

  return nullptr;
}

bool tcalclang::isNameChar1( char c )
{
  if( c >= '@' && c <= 'Z' ) {
    return true;
  }
  if( c >= 'a' && c <= 'z' ) {
    return true;
  }
  if( c == '_' || c == '$' ) {
    return true;
  }
  return false;
}

bool tcalclang::isNameChar( char c )
{
  if( c >= '0' && c <= '9' ) {
    return true;
  }
  return tcalclang::isNameChar1( c );
}

bool tcalclang::checkNameOnly( const char *s )
{
  if( !s ) {
    return false;
  }
  if( ! isNameChar1( *s ) ) {
    return false;
  }
  ++s;
  for( ; *s; ++s ) {
    if( ! isNameChar( *s )  ) {
      return false;
    }
  }
  return true;
}

bool tcalclang::checkName( const char *s, char *d, unsigned bname_max, int *idx )
{
  if( !s ) {
    return false;
  }
  skip_ws( s );

  unsigned b_len = 0;
  if( ! isNameChar1( *s ) ) {
    return false;
  }

  if( d && b_len < bname_max-1 ) {
    d[b_len++] = *s;
  }
  ++s;

  for( ; *s; ++s ) {
    if( d && b_len >= bname_max-1 ) {
      return false;
    }
    if( ! isNameChar( *s )  ) {
      break;
    }
    if( d ) {
      d[b_len++] = *s;
    }
  }

  if( d ) {
    d[b_len] = '\0';
  }

  skip_ws( s );

  if( *s == '\0' ) { // only name
    if( idx ) {
      *idx = 0;
    }
    return true;
  }

  if( *s != '[' ) {
    return false;
  }

  ++s;
  skip_ws( s );
  int v = 0, nd = 0;

  // convert decimal index
  for( ; *s >= '0' && *s <= '9'; ++s,++nd ) {
    v = v * 10 + ( *s - '0' );
  }

  skip_ws( s );
  if( *s != ']' || nd < 1 ) {
    return false;
  }
  ++s;

  skip_ws( s );
  if( *s != '\0' && *s != ';' ) {
    return false;
  }

  if( idx ) {
    *idx = v;
  }

  return true;
}


// ------------------------- DataInfo ---------------------------

DataInfo::DataInfo( float *pv, const char *nm, uint32_t sz )
  : d_f( pv ),  name( nm ), n( sz ), dtype( DataType::t_float )
{
}

DataInfo::DataInfo( int   *pv, const char *nm, uint32_t sz )
  : d_i( pv ),  name( nm ), n( sz ), dtype( DataType::t_int   )
{
}

// ------------------------- Datas ---------------------------

int Datas::addDatas( float *pv, const char *nm, uint32_t sz )
{
  if( ! checkNameOnly( nm ) ) {
      return 0;
  }
  if( findData( nm ) ) {
    return 0;
  }
  d.emplace_back( pv, nm, sz );
  return d.size();
}

int Datas::addDatas( int   *pv, const char *nm, uint32_t sz )
{
  if( ! checkNameOnly( nm ) ) {
      return 0;
  }
  if( findData( nm ) ) {
    return 0;
  }
  d.emplace_back( pv, nm, sz );
  return d.size();
}

const DataInfo* Datas::findData( const char *nm ) const
{
  if( ! checkNameOnly( nm ) ) {
      return nullptr;
  }
  // slow, but good for MC
  auto p = find_nm( nm );
  if( p != d.end() ) {
    return &(*p);
  }
  return nullptr;
}



void* Datas::ptr( const char *nmi, DataType &dt ) const
{
  char nm1[max_nm_expr_len];
  int idx = 0;
  dt = DataType::t_none;
  bool ok = checkName( nmi, nm1, sizeof(nm1), &idx );
  if( !ok ) {
    return nullptr;
  }

  auto p = find_nm( nm1 );
  if( p == d.end() ) {
    return nullptr;
  }
  if( idx >= int(p->n) ) {
    return nullptr;
  }
  switch( p->dtype ) {
    case DataType::t_int:
      dt = DataType::t_int;
      return (p->d_i) + idx;
    case DataType::t_float:
      dt = DataType::t_float;
      return (p->d_f) + idx;
    default: break;
  }
  return nullptr;
}

int* Datas::ptr_i( const char *nmi ) const
{
  DataType dt;
  void *p = ptr( nmi, dt );
  if( !p || dt != DataType::t_int ) {
    return nullptr;
  }
  return static_cast<int*>( p );
}

float* Datas::ptr_f( const char *nmi ) const
{
  DataType dt;
  void *p = ptr( nmi, dt );
  if( !p || dt != DataType::t_int ) {
    return nullptr;
  }
  return static_cast<float*>( p );
}

float Datas::val( const char *nmi ) const
{
  DataType dt;
  void *p = ptr( nmi, dt );
  if( !p  ) {
    return 0;
  }
  switch( dt ) {
   case DataType::t_int :
     return (float)(*(int*)p);
   case DataType::t_float :
     return *(float*)p;
   default: break;
  }
  return 0;
}

int Datas::set( const char *nmi, const char *sval )
{
  DataType dt;
  void *p = ptr( nmi, dt );
  if( !p  ) {
    return 0;
  }

  char *eptr;
  switch( dt ) {
   case DataType::t_int :
     {
       int v = strtol( sval, &eptr, 0 );
       if( *eptr == '\0' ) {
         (*(int*)p) = v;
       }
     }
     break;
   case DataType::t_float :
     {
       float v = strtof( sval, &eptr );
       if( *eptr == '\0' ) {
         (*(float*)p) = v;
       }
     }
     break;
   default: return 0;
  }
  return 1;
}

int Datas::parse_arg( const char *s, Cmd &cmd ) const
{
  if( !s ) {
    return 0;
  }
  skip_ws( s );
  if( *s == '\0' ) {
    return 0;
  }

  char *eptr;
  int v_i = strtol( s, &eptr, 0 );
  if( *eptr == '\0' || *eptr == ';' || *eptr == '#' ) {
    cmd = Cmd( v_i );
    return 1;
  }
  float v_f = strtof( s, &eptr );
  if( *eptr == '\0' || *eptr == ';' || *eptr == '#' ) {
    cmd = Cmd( v_f );
    return 1;
  }

  if( ! isNameChar1( *s ) ) {
    // cerr << "... Bad first char '" << *s << '\'' << endl;
    return 0;
  }
  string nm0; nm0.reserve( 64 ); // pure name w/o index
  nm0 += *s++;

  for( ; *s; ++s ) {
    if( ! isNameChar( *s )  ) {
      break;
    }
    nm0 += *s;
  }

  skip_ws( s );

  int idx = 0;
  if( ! ( *s == '\0' || *s == ';' || *s == '#'  ) ) { // not only name

    if( *s != '[' ) {
      // cerr << "... [ required: '" << s << '"' << endl;
      return 0;
    }

    ++s;
    skip_ws( s );

    // convert index
    idx = strtol( s, &eptr, 0 );
    if( eptr == s ) {
      // cerr << "... bad index '" << s << '"' << endl;
      return 0;
    }
    s = eptr;

    skip_ws( s );
    if( *s != ']' ) {
      // cerr << "... ] required: '" << s << '"' << endl;
      return 0;
    }
    ++s;

    skip_ws( s );
    if( *s != '\0' && *s != ';' ) {
      // cerr << "... strange tail found: \"" << s << '"' << endl;
      return 0;
    }
  }

  auto p = find_nm( nm0.c_str() );
  if( p == d.end() ) {
    // cerr << "... name not found: \"" << nm0 << '"' << endl;
    return 0;
  }

  if( idx >= int(p->n) ) {
    // cerr << "... bad index " << idx <<  endl;
    return 0;
  }

  switch( p->dtype ) {
    case DataType::t_int:
      cmd = Cmd( (p->d_i) + idx );
      return 1;
    case DataType::t_float:
      cmd = Cmd( (p->d_f) + idx );
      return 1;
    default: break;
  }

  // cerr << "... unknown type " << int(p->dtype) <<  endl;
  return 0;
}

void Datas::list() const
{
  STDOUT_os;
  for( auto v :d ) {
    os << v.name << ' ' << dtype2name( v.dtype ) << ' ' << v.n << NL;
  }
}

void Datas::dump( const char *nm ) const
{
  STDOUT_os;
  if( !nm || !*nm ) {
    for( auto p : d ) {
      os << "# " << p.name << "= [ ";
      dump_inner( &p );
    }
    return;
  }

  os << "#  " << nm << " = [ ";

  auto p = find_nm( nm );
  if( p == d.end() ) {
    os << " not_found ]" << NL;
  } else {
    dump_inner( &(*p) );
  }
}

void Datas::dump_inner( const DataInfo *p ) const
{
  if( !p ) {
    return;
  }
  STDOUT_os;

  if( p->dtype == DataType::t_int ) {
    const int *x = p->d_i;
    for( unsigned i=0; i<p->n; ++i ) {
      os << x[i] << ' ';
    }
  } else if( p->dtype == DataType::t_float ) {
    const float *x = p->d_f;
    for( unsigned i=0; i<p->n; ++i ) {
      os << x[i] << ' ';
    }
  } else {
    os << " ?type_ " << int(p->dtype );
  }
  os << "] " << dtype2name( p->dtype ) << " [" << p->n << "] " << NL;
}

// ------------------------ Engine -------------------------------
void Engine::clear() {
  pgm.clear();
  x = y = z = tmp = 0;
}

int Engine::parseCmd( const char *s, Cmd &cmd )
{
  if( !s ) {
    return 0;
  }
  skip_ws( s );
  STDOUT_os;

  const char *eptr;
  auto fi = findFunc( s, &eptr );
  if( fi ) {
    // cout << "# debug: found function \"" << fi->nm << "\" " << fi->narg << endl;
    switch( fi->narg ) {
      case 0: cmd.op = CmdOp::fun0; break;
      case 1: cmd.op = CmdOp::fun1; break;
      case 2: cmd.op = CmdOp::fun2; break;
      case 3: cmd.op = CmdOp::fun3; break;
      default:
              // cerr << "# err: bad func argns number " << fi->narg << endl;
              return 0;
    }
    cmd.d_i = reinterpret_cast<int*>( fi->ptr );
    return 1;
  }

  auto oi = findCmdOp( s, &eptr );
  if( !oi ) {
    os << "# err: unknown cmd \"" << s << '\"' << NL;
    return 0;
  }
  s = eptr;

  if( oi->narg > 0 ) {
    skip_ws( s );
    int rc = datas.parse_arg( s, cmd );
    if( !rc ) {
      // cerr << ".. fail to parse arg: \"" << s << "\" " << endl;
      return 0;
    }
  }
  cmd.op = oi->op;

  return 1;
}

int Engine::addCmd( const char *s )
{
  Cmd cmd;
  if( !parseCmd( s, cmd ) ) {
    return 0;
  }
  pgm.emplace_back( cmd );

  return pgm.size();
}

int Engine::execCmd( Cmd cmd )
{
  int rc = 1;
  auto op = cmd.op;
  // cout << "# debug execCmd '" << op << "' " << x << ' ';
  switch( op ) {
    case CmdOp::exit:  rc = 0;  break;
    case CmdOp::err:   rc = 0;  break; // TODO: err code
    case CmdOp::load:  rc = load( cmd ); x  = tmp; break;
    case CmdOp::loady: rc = load( cmd ); y  = tmp; break;
    case CmdOp::loadz: rc = load( cmd ); z  = tmp; break;
    case CmdOp::stor:  rc = stor( cmd );           break;
    case CmdOp::add:   rc = load( cmd ); x += tmp; break;
    case CmdOp::sub:   rc = load( cmd ); x -= tmp; break;
    case CmdOp::mul:   rc = load( cmd ); x *= tmp; break;
    case CmdOp::div:   rc = load( cmd ); x /= tmp; break;
    case CmdOp::fun0:
                     x = (reinterpret_cast<Func0Arg>( cmd.d_i ))();
                     break;
    case CmdOp::fun1:
                     x = (reinterpret_cast<Func1Arg>( cmd.d_i ))( x );
                     break;
    case CmdOp::fun2:
                     x = (reinterpret_cast<Func2Arg>( cmd.d_i ))( x, y );
                     break;
    case CmdOp::fun3:
                     x = (reinterpret_cast<Func3Arg>( cmd.d_i ))( x, y, z );
                     break;
    case CmdOp::pi:
                     x = (float)(3.141592654);
                     break;
    default:
                     // cerr << "# err: Unknown command " << (char)(op) << endl;
                     rc = 0; break;
  }
  // cout << x << " rc= " << rc << endl;
  return rc;
}

int Engine::exec()
{
  int rc = 0;
  for( auto c : pgm ) {
    rc = execCmd( c );
    if( rc == 0 ) {
      break;
    }
  };
  return rc;
}

int Engine::execCmdStr( const char *s )
{
  Cmd cmd;
  if( !parseCmd( s, cmd ) ) {
    return 0;
  }
  return execCmd( cmd );
}


int Engine::load( const Cmd &c )
{
  auto dt = c.dtype;
  switch( dt ) {
    case DataType::t_int:
      if( ! c.d_i ) {
        return 0;
      }
      tmp = *(c.d_i); break;
    case DataType::t_float:
      if( ! c.d_f ) {
        return 0;
      }
      tmp  = *(c.d_f); break;
    case DataType::t_int_i:
      tmp = c.i; break;
    case DataType::t_float_i:
      tmp  = c.f; break;
    default:
      return 0;
  }
  return 1;
}

int Engine::stor( const Cmd &c )
{
  auto dt = c.dtype;
  switch( dt ) {
    case DataType::t_int:
      if( ! c.d_i ) {
        return 0;
      }
      *(c.d_i) = int(x); break;
    case DataType::t_float:
      if( ! c.d_f ) {
        return 0;
      }
      *(c.d_f) = x; break;
    default:
      return 0;
  }
  return 1;
}

