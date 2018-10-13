#ifndef _TCALCLANG_H
#define _TCALCLANG_H

#include <stdint.h>
#include <vector>
#include <algorithm>
#include <cstring>

namespace tcalclang {

  enum class DataType : uint8_t {
    t_none  = 0,
    t_int   = 2,   t_int_i = 3,
    t_float = 4, t_float_i = 5,
    t_bad = 255 // + const
  };
  const char* dtype2name( DataType dt );
  inline void skip_ws( const char *& s ) { for( ; *s == ' ' ; ++s ) { /* NOP */ } }

  const int max_nm_expr_len = 128;
  bool isNameChar1( char c ); // first char only
  bool isNameChar( char c );
  bool checkNameOnly( const char *s );
  bool checkName( const char *s, char *d, unsigned bname_max, int *idx );

  enum class CmdOp : uint8_t {
    exit  = 0,
    err   = 'E',
    nop   = '.',
    add   = '+',
    sub   = '-',
    mul   = '*',
    div   = '/',
    mod   = '%',
    stor  = 'S',
    load  = 'L',
    loady = 'y',
    loadz = 'z',
    loadn = 'n',
    fun0 =  '0',
    fun1 =  '1',
    fun2 =  '2',
    fun3 =  '3',
    pi   =  'p',
  };

  struct CmdOpInfo {
    CmdOp op;
    int narg; // <0 - number of function arguments
    const char *nm;
  };
  extern const CmdOpInfo cmdOpInfos[];
  const CmdOpInfo* findCmdOp( const char *s, const char** eptr );

  typedef float (*Func0Arg)();
  typedef float (*Func1Arg)( float );
  typedef float (*Func2Arg)( float, float );
  typedef float (*Func3Arg)( float, float, float );
  struct FuncInfo {
    const char *nm;
    void *ptr;
    int narg;
  };
  extern const FuncInfo funcInfos[];
  inline const unsigned max_func_name_length = 32;
  const FuncInfo* findFunc( const char *s, const char** eptr );


  struct DataInfo {
    union {
      float *d_f;
      int   *d_i;
    };
    const char *name; // owning only if dyn - not now
    uint32_t n;
    DataType dtype = DataType::t_bad;
    bool dyn = false;
    //
    DataInfo( float *pv, const char *nm, uint32_t sz = 1 );
    DataInfo( int   *pv, const char *nm, uint32_t sz = 1 );
    // ~DataInfo();
  };


  struct Cmd {
    union {
      float *d_f;
      int   *d_i;
      float f;
      int   i;
    };
    // uint8_t op = 'E'; // error
    CmdOp op = CmdOp::err;
    DataType dtype = DataType::t_bad;
    Cmd() = default;
    Cmd(   int   v ) :   i(  v ), dtype( DataType::t_int_i   ) {};
    Cmd( float   v ) :   f(  v ), dtype( DataType::t_float_i ) {};
    Cmd(   int *pv ) : d_i( pv ), dtype( DataType::t_int     ) {};
    Cmd( float *pv ) : d_f( pv ), dtype( DataType::t_float   ) {};
  };

  class Datas {
    public:
     Datas() = default;
     int addDatas( float *pv, const char *nm, uint32_t sz = 1 );
     int addDatas( int   *pv, const char *nm, uint32_t sz = 1 );
     const DataInfo* findData( const char *nm ) const;
     // int* ptr_i( const char *nmi ) const; // nmi = name or name[idx]
     // float* ptr_f( const char *nmi ) const;
     float val( const char *nmi ) const;
     int set( const char *nmi, const char *sval );
     int parse_arg( const char *s, Cmd &cmd ) const;
     // for debug
     void list() const;
     int dump( const char *nm ) const;
    protected:
     void* ptr( const char *nmi , DataType &dt ) const;
     std::vector<DataInfo> d;
     std::vector<DataInfo>::const_iterator find_nm( const char *nm ) const
       { return find_if( d.begin(), d.end(), [nm](auto &v) { return ( std::strcmp( nm, v.name ) == 0 ); }  ); };
     void dump_inner( const DataInfo *p ) const;
  };


#define ADD_DATAS(d) datas.addDatas( d, #d, size(d) )
#define ADD_DATA(d)  datas.addDatas( &d, #d, 1 )
#define ADD_DATAS_NM(d,name) datas.addDatas( d, name, size(d) )
#define ADD_DATA_NM(d,name)  datas.addDatas( d, name, 1 )



  class Engine
  {
    public:
     Engine( Datas &d ) : datas( d ) { clear(); };
     int parseCmd( const char *s, Cmd &cmd );
     int addCmd( const char *s );
     int pgmSize() const { return pgm.size(); };
     void clear();
     int exec();
     int execCmd( Cmd cmd );
     int execCmdStr( const char *s );
     void dumpState() const;

    protected:
     int load( const Cmd &c ); // to tmp
     int stor( const Cmd &c ); // from x
     std::vector<Cmd> pgm;
     float x, y, z, tmp;
     Datas &datas;
  };



};


#endif

