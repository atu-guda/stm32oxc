// #include <iostream>
#include <algorithm>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <cmath>

#include <oxc_auto.h>
#include <oxc_miscfun.h>
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
  //{ CmdOp::loadn ,  1 , "LN"    } ,
  { CmdOp::fun0  ,  0 , ""      } ,
  { CmdOp::fun1  , -1 , ""      } ,
  { CmdOp::fun2  , -2 , ""      } ,
  { CmdOp::fun3  , -3 , ""      } ,
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

DataInfo::DataInfo( float *pv, const char *nm, uint32_t sz, bool isRo )
  : d_f( pv ),  name( nm ), n( sz ), dtype( DataType::t_float ), ro ( isRo )
{
}

DataInfo::DataInfo( int   *pv, const char *nm, uint32_t sz, bool isRo )
  : d_i( pv ),  name( nm ), n( sz ), dtype( DataType::t_int ), ro ( isRo )
{
}

// ------------------------- Datas ---------------------------

int Datas::checkAtAdd( const char *nm ) const
{
  if( ! checkNameOnly( nm ) ) {
    return 0;
  }
  if( findData( nm ) ) {
    return 0;
  }
  return 1;
}

int Datas::addDatas( float *pv, const char *nm, uint32_t sz )
{
  if( ! checkAtAdd( nm )  ) {
    return 0;
  }
  d.emplace_back( pv, nm, sz, false );
  return d.size();
}

int Datas::addDatas( int   *pv, const char *nm, uint32_t sz )
{
  if( ! checkAtAdd( nm )  ) {
    return 0;
  }
  d.emplace_back( pv, nm, sz, false );
  return d.size();
}

int Datas::addDatasRo( const float *pv, const char *nm, uint32_t sz )
{
  if( ! checkAtAdd( nm )  ) {
    return 0;
  }
  d.emplace_back( const_cast<float*>(pv), nm, sz, true );
  return d.size();
}

int Datas::addDatasRo( const int   *pv, const char *nm, uint32_t sz )
{
  if( ! checkAtAdd( nm )  ) {
    return 0;
  }
  d.emplace_back( const_cast<int*>(pv), nm, sz, true );
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

const DataInfo* Datas::findPtr( const void *ptr, int &idx ) const
{
  static_assert( sizeof(int) == sizeof(float) ); // TODO sizeof in DataInfo
  for( const auto &di : d ) {
    ptrdiff_t st = (ptrdiff_t)(di.d_i);
    if( (ptrdiff_t)(ptr) < st ) {
      continue;
    }
    ptrdiff_t ofs = ( (ptrdiff_t)(ptr) - st ) / sizeof(float); // here
    if( ofs < (ptrdiff_t)di.n ) {
      idx = (int)(ofs);
      return &di;
    }
  }
  return nullptr;
}



void* Datas::ptr( const char *nmi, DataType &dt, bool rw ) const
{
  char nm1[max_nm_expr_len];
  int idx = 0;
  dt = DataType::t_none;
  bool ok = checkName( nmi, nm1, sizeof(nm1), &idx );
  if( !ok ) {
    return nullptr;
  }

  auto p = find_nm( nm1 );
  if( p == d.end()  ||  idx >= int(p->n) ||  ( rw && p->ro ) ) {
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


float Datas::val( const char *nmi ) const
{
  DataType dt;
  void *p = ptr( nmi, dt, false );
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
  void *p = ptr( nmi, dt, true );
  if( !p ) {
    return 0;
  }

  char *eptr;
  int rc = 0;
  switch( dt ) {
   case DataType::t_int :
     {
       int v = strtol( sval, &eptr, 0 );
       if( *eptr == '\0' || *eptr == ' ' || *eptr == ';'  || *eptr == '#' ) {
         (*(int*)p) = v;
         rc = 1;
       }
     }
     break;
   case DataType::t_float :
     {
       float v = strtof( sval, &eptr );
       if( *eptr == '\0' || *eptr == ' ' || *eptr == ';'  || *eptr == '#' ) {
         (*(float*)p) = v;
         rc = 1;
       }
     }
     break;
   default: return 0;
  }
  return rc;
}

int Datas::parse_arg( const char *s, Cmd &cmd ) const
{
  STDOUT_os;
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
    os << "# err:  Bad first char '" << *s << '\'' << NL;
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
      os << "# err: ... [ required: \"" << s << '"' << NL;
      return 0;
    }

    ++s;
    skip_ws( s );

    // convert index
    idx = strtol( s, &eptr, 0 );
    if( eptr == s ) {
      os << "# err: bad index \"" << s << '"' << NL;
      return 0;
    }
    s = eptr;

    skip_ws( s );
    if( *s != ']' ) {
      os << "# err:  ] required: \"" << s << '"' << NL;
      return 0;
    }
    ++s;

    skip_ws( s );
    if( *s != '\0' && *s != ';' ) {
      os << "# err:  strange tail found: \"" << s << '"' << NL;
      return 0;
    }
  }

  auto p = find_nm( nm0.c_str() );
  if( p == d.end() ) {
    os << "# err: name not found: \"" << nm0.c_str() << '"' << NL;
    return 0;
  }

  if( idx >= int(p->n) ) {
    os << "# err: bad index " << idx <<  NL;
    return 0;
  }

  if( cmd.op == CmdOp::stor  &&  p->ro ) {
    os << "# err: R/O \"" << nm0.c_str() << '"' <<  NL;
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

  // cerr << "... unknown type " << int(p->dtype) <<  NL;
  return 0;
}

void Datas::list() const
{
  STDOUT_os;
  for( auto v :d ) {
    os << v.name << ' ' << dtype2name( v.dtype ) << ' ' << v.n << NL;
  }
}

int Datas::dump( const char *nm ) const
{
  STDOUT_os;
  if( !nm || !*nm ) {
    for( auto p : d ) {
      os << "# " << p.name << "= [ ";
      dump_inner( &p );
    }
    return 1;
  }

  // TODO: single val[idx]

  auto p = find_nm( nm );
  if( p != d.end() ) {
    os << "#  " << nm << " = [ ";
    dump_inner( &(*p) );
    return 1;
  }

  DataType dt;
  void *pv = ptr( nm, dt, false );
  if( pv ) {
    os << "#  " << nm << " =  ";
    switch( dt ) {
      case DataType::t_int :
            os << (*(int*)pv);
            break;
      case DataType::t_float :
            os << (*(float*)pv);
            break;
      default:
            os << "Unknown type " << (int)dt << NL;
            return 0;
    }

    return 1;
  }

  os << "# error: \"" << nm << "\" not found" << NL;
  return 0;
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
  os << "] " << dtype2name( p->dtype ) << " [" << p->n << "] "
     << ( (p->ro) ? "ro" : "" ) << NL;
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
    // cout << "# debug: found function \"" << fi->nm << "\" " << fi->narg << NL;
    switch( fi->narg ) {
      case 0: cmd.op = CmdOp::fun0; break;
      case 1: cmd.op = CmdOp::fun1; break;
      case 2: cmd.op = CmdOp::fun2; break;
      case 3: cmd.op = CmdOp::fun3; break;
      default:
              os << "# err: bad func argns number " << fi->narg << NL;
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

  cmd.op = oi->op; // for correct ro in parse_arg
  if( oi->narg > 0 ) {
    skip_ws( s );
    int rc = datas.parse_arg( s, cmd );
    if( !rc ) {
      os << "# err: fail to parse arg: \"" << s << "\" " << NL;
      return 0;
    }
  }
  cmd.op = oi->op; // restore after parse_arg

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

int Engine::execCmd( const Cmd &cmd )
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
    default:
                     // cerr << "# err: Unknown command " << (char)(op) << NL;
                     rc = 0; break;
  }
  // cout << x << " rc= " << rc << NL;
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

void Engine::dumpState() const
{
  STDOUT_os;
  os << "# state: x= " << x << " y= " << y << " z= " << z << " tmp= " << tmp << NL;
}

void Engine::listPgm() const
{
  STDOUT_os;
  os << "# program: " << NL;
  int n = 0;
  for( const auto &c : pgm ) {
    os << "# " << n << ' ';
    dumpCmd( c );
    ++n;
    os << NL;
  };
}

void Engine::dumpCmd( const Cmd &c ) const
{
  STDOUT_os;

  const CmdOpInfo *poi = nullptr;
  for( const auto &oi : cmdOpInfos ) {
    if( c.op == oi.op ) {
      poi = &oi;
      os << oi.nm;
      break;
    }
  }
  if( ! poi ) {
    os << "??? " << (int)(c.op) << NL;
    return;
  }

  // function: special case
  if(    poi->op == CmdOp::fun0 || poi->op == CmdOp::fun1
      || poi->op == CmdOp::fun2 || poi->op == CmdOp::fun3 ) {
    bool fun_found = false;
    for( const auto &fi : funcInfos ) {
      if( (void*)(c.d_i) == (void*)(fi.ptr) ) {
        os << fi.nm; fun_found = true;
        break;
      }
    }
    if( ! fun_found ) {
      os << "Unknown_fun";
    }
    return; // no
  }

  if( poi->narg != 1 ) {
    return; // all non-one arg: special case
  }

  // os << " 0x" << HexInt( c.d_i );
  os << ' ';
  auto dt = c.dtype;
  switch( dt ) {
    case DataType::t_int:
    case DataType::t_float:
      dumpArg( c );
      break;
    case DataType::t_int_i:
      os << c.i; break;
    case DataType::t_float_i:
      os << c.f; break;
    default:
      os <<  "?type " << (int)(dt); break;
  }
}

void Engine::dumpArg( const Cmd &c ) const
{
  STDOUT_os;
  int idx = 0;
  if( auto di = datas.findPtr( c.d_i, idx ) ) {
    os << di->name;
    if( di->n > 1 ) {
      os << '[' << idx << ']';
    }
  }
  os << " ;   0x" << HexInt( c.d_i ) << ' ' << dtype2name( c.dtype );
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

