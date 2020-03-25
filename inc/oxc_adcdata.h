// =====================================================================================
//
//       Filename:  oxc_adcdata.h
//
//    Description:  Common data structures and function to store and handle ADC-alike data
//
//        Created:  2020-03-18 17:08:00
//
//         Author:  Anton Guda (atu), atu@nmetau.edu.ua
//
// =====================================================================================
#ifndef _OXC_ADCDATA_H
#define _OXC_ADCDATA_H

#include <type_traits>
#include <limits>
#include <algorithm>
#include <cstdlib>

#include <oxc_floatfun.h>

template< int N, typename FL = float >
class AdcData {
  public:
   using TS =
     typename std::conditional<         N < -16, int32_t,
       typename std::conditional<       N <  -8, int16_t,
         typename std::conditional<     N <   0, int8_t,
           typename std::conditional<   N <=  8, uint8_t,
             typename std::conditional< N <= 16, uint16_t,
               uint32_t
           >::type
         >::type
       >::type
     >::type
   >::type;

   using malloc_fun_t = void* (*)(std::size_t);
   using free_fun_t   = void  (*)(void*);

   static constexpr unsigned max_ch = 16;

   AdcData( malloc_fun_t a_malloc_fun = BOARD_ADC_MALLOC, free_fun_t a_free_fun = BOARD_ADC_FREE ) noexcept
     : bpv( abs(N) ),
     sign( std::is_signed<TS>::value ),
     max_val( 0xFFFFFFFFu >> (32-bpv+sign) ),
     malloc_fun( a_malloc_fun ),
     free_fun( a_free_fun )
     {
       static_assert( N >= -32 && N <= 32 && N != 0 , "Unsupported storage size");
       for( auto &x : col_mult ) { x = 1.0f; };
     } ;
   AdcData( const AdcData &rhs ) = delete;
   ~AdcData() { free(); };
   AdcData& operator=( const AdcData &rhs ) = delete;
   bool alloc( unsigned a_n_col, unsigned a_n_row, unsigned row_add = 0 );
   void free();
   unsigned get_n_row() const { return n_row; }
   unsigned get_n_col() const { return n_col; }
   TS* data() { return d; }
   const TS* cdata() const { return d; }
   const TS* c_row( unsigned i_row ) const { return d + i_row * n_col;  } // no check here
   TS* row( unsigned i_row ) { return d + i_row * n_col; }
   bool is_signed() const { return sign; }
   uint8_t get_bpv() const { return bpv; }
   unsigned size_row() const { return sz_row; }
   unsigned size_all() const { return sz_all; }
   int get_v_ref_uV() const { return v_ref_uV; }
   int get_max_val() const { return max_val; }
   void set_v_ref_uV( unsigned v_ref ) { v_ref_uV = v_ref; }
   FL get_v_ref() const { return v_ref_uV * 1e-6f; }
   FL get_d_t() const { return d_t; }
   void set_d_t( FL a_d_t )  { d_t = a_d_t; }
   inline unsigned rowcol_idx( unsigned i_row, unsigned i_col ) const { return i_row * n_col + i_col; };
   TS operator()( unsigned i_row, unsigned i_col ) const { return d[ rowcol_idx( i_row, i_col ) ]; };
   TS& operator()( unsigned i_row, unsigned i_col ) { return d[ rowcol_idx( i_row, i_col )  ]; };
   FL v( unsigned i_row, unsigned i_col ) const {
     return (FL)(v_ref_uV) * col_mult[i_col] * (FL)(1e-6f) * d[ rowcol_idx( i_row, i_col ) ] / max_val;
   };
   FL get_col_mult( unsigned i_col ) const { return (i_col <= n_col) ? col_mult[i_col] : 0; };
   void set_col_mult( unsigned i_col, FL mult ) { if (i_col <= n_col) { col_mult[i_col] = mult; } };
   int out_float( OutStream &os, unsigned st = 0, unsigned n = 0x3FFFFFFF ) const;
   int out_hex( OutStream &os, unsigned st = 0, unsigned n = 0x3FFFFFFF ) const;
   int out_any( OutStream &os, bool isHex, unsigned st = 0, unsigned n = 0x3FFFFFFF ) const;
   int out_header( OutStream &os, unsigned st = 0, unsigned n = 0x3FFFFFFF ) const;
   void set_out_fmt( unsigned a_out_w, unsigned a_out_prec = 99 ) { out_w = a_out_w; out_prec = a_out_prec; }
  private:
   const uint8_t bpv; //* bit per value
   const bool sign;   //* treat values as signed
   const int max_val;
   const malloc_fun_t malloc_fun;
   const free_fun_t   free_fun;
   unsigned n_col = 0, n_row = 0;
   unsigned sz_row = 0, sz_all = 0;
   int v_ref_uV = 1000000; // reference value in uV
   unsigned out_w = 11, out_prec = 99;
   TS* d = nullptr;
   FL col_mult[max_ch];
   FL d_t = 1.0f;
};

template< int N, typename FL  >
bool AdcData<N,FL>::alloc( unsigned a_n_col, unsigned a_n_row, unsigned row_add  )
{
  if( a_n_col == 0 || a_n_row == 0 ) {
    free();
    return false;
  }

  if( n_col == a_n_col  && n_row == a_n_row ) {
    return true;
  }
  free();

  sz_row = a_n_col * sizeof(TS);
  sz_all = sz_row * a_n_row;
  auto sz_all_full = sz_all + row_add * sz_row;
  // cerr <<  "sz_row= " << sz_row << " sz_all_full= " << sz_all_full << NL;
  d = static_cast<TS*>( malloc_fun( sz_all_full ) );
  if( !d ) {
    return false;
  }
  std::fill( d, d+(n_row+row_add)*n_col, 0 );
  n_col = a_n_col; n_row = a_n_row;
  // cerr << "d= " << d << " sz_row= " << sz_row << " sz_all_full= " << sz_all_full << NL;
  return true;
}

template< int N, typename FL  >
void AdcData<N,FL>::free()
{
  if( d ) {
    free_fun( d ); d = nullptr;
  }
  n_col = n_row = sz_row = sz_all = 0;
}

template< int N, typename FL  >
int AdcData<N,FL>::out_float( OutStream &os, unsigned st, unsigned n ) const
{
  out_header( os, st, n );

  unsigned en = st + n; // TODO: fox overflow
  if( en > n_row ) {
    en = n_row;
  }

  break_flag = 0;
  for( unsigned r = st;  r < en && !break_flag;  ++r ) {
    FL t = r * d_t;
    os <<  XFmt( t, cvtff_auto, 14, 6 );

    for( unsigned c = 0; c < n_col; ++c ) {
      os << ' ' << XFmt( v( r, c ), cvtff_auto, out_w, out_prec );
    }
    os << NL;
  }

  return 1;
}

template< int N, typename FL  >
int AdcData<N,FL>::out_hex( OutStream &os, unsigned st, unsigned n ) const
{
  out_header( os, st, n );

  unsigned en = st + n; // TODO: fix overflow
  if( en > n_row ) {
    en = n_row;
  }
  for( unsigned r = st;  r < en;  ++r ) {
    // os << HexInt( r );
    for( unsigned c = 0; c < n_col; ++c ) {
      if constexpr ( sizeof(TS) > 16 ) {
        os << HexInt(   operator()( r, c ) ) << ' ';
      } else {
        os << HexInt16( operator()( r, c ) ) << ' ';
      }
    }
    os << NL;
  }

  return 1;
}

template< int N, typename FL  >
int AdcData<N,FL>::out_any( OutStream &os, bool isHex, unsigned st, unsigned n ) const
{
  if( isHex ) {
    return out_hex( os, st, n );
  }
  return out_float( os, st, n );
}

template< int N, typename FL  >
int AdcData<N,FL>::out_header( OutStream &os, unsigned st, unsigned n ) const
{
  os << "## AdcData "    << N        << NL;
  os << "#@ max_val= "   << max_val  << NL;
  os << "#@ bpv= "       << (int)bpv << NL;
  os << "#@ sign= "      << sign     << NL;
  os << "#@ n_row= "     << n_row    << NL;
  os << "#@ n_col= "     << n_col    << NL;
  os << "#@ st= "        << st       << NL;
  os << "#@ n= "         << n        << NL;
  os << "#@ d_t= "       << d_t      << NL;
  os << "#@ v_ref_uV= "  << v_ref_uV << NL;
  for( unsigned i=0; i<n_col; ++i ) {
    os << "#@ k_" << i << "= " << col_mult[i] << NL;
  }
  return 1;
}

// ---------------------------------------------------------------------------------

struct StatIntChannel {
  long long
    vmin = std::numeric_limits<long long>::max(),
    vmax = std::numeric_limits<long long>::min(),
    sum = 0, sum2 = 0;
  unsigned n = 0;
  xfloat scale = (1.0f / 0x7FFF ), mean = 0, mean2 = 0, sd = 0;
  xfloat fmin = 0, fmax = 0, fsum = 0, fsum2 = 0;
  explicit StatIntChannel( xfloat a_scale = 1.0f ) : scale( a_scale ) {} ;
  void set_scale( xfloat a_scale ) { scale = a_scale; }
  void reset() {
    vmin = std::numeric_limits<long long>::max();
    vmax = std::numeric_limits<long long>::min();
    sum = sum2 = 0 ;
    n = 0;
    mean = mean2 = sd = 0;
    fmin = fmax = fsum = fsum2 = 0;
  }
  void add( int v );
  void calc();
};

struct StatIntData {
  static const constexpr unsigned max_n_ch = 16;

  struct StructPart {
    const xfloat StatIntChannel::* pptr;
    const char* const label;
  };

  StatIntChannel d[max_n_ch];
  unsigned n = 0;
  unsigned n_ch = 0;

  //
  StatIntData( unsigned nch, xfloat a_scale ); // slurp AdcData?
  template< int N, typename FL = float >
    explicit StatIntData( const AdcData<N,FL> &adc );
  auto getNch() const { return n_ch; }
  void add( const int *v );
  void reset();
  void calc();
  template< int N, typename FL = float >
    void slurp( const AdcData<N,FL> &adc );
  void out_part( OutStream &os, const xfloat StatIntChannel::* pptr, const char *lbl ) const;
  void out_parts( OutStream &os ) const;
  //
  static const constexpr StructPart structParts[] = {
    { &StatIntChannel::mean,  "mean " },
    { &StatIntChannel::fmin,  "min  " },
    { &StatIntChannel::fmax,  "max  " },
    // { &StatIntChannel::fsum,  "sum  " },
    // { &StatIntChannel::fsum2, "sum2 " },
    { &StatIntChannel::mean2, "mean2" },
    { &StatIntChannel::sd,    "sd   " }
  };
};

inline OutStream& operator<<( OutStream &os, const StatIntData &sd ) { sd.out_parts( os );  return os; } ;

template< int N, typename FL = float >
  StatIntData::StatIntData( const AdcData<N,FL> &adc )
  : n_ch( adc.get_n_col() )
{
  FL c0 = adc.get_v_ref() / adc.get_max_val();
  for( unsigned i=0 ; i<n_ch; ++i ) {
    d[i].set_scale( c0 * adc.get_col_mult( i ) );
  }
  reset();
}

template< int N, typename FL = float >
  void StatIntData::slurp( const AdcData<N,FL> &adc )
{
  const unsigned nr = adc.get_n_row();
  const unsigned nc = adc.get_n_col();
  if( nc != n_ch ) {
    return; // ERROR!
  }
  int vv[nc];
  for( unsigned r=0; r < nr; ++r ) {
    for( unsigned c=0; c<nc; ++c ) {
      vv[c] = adc( r, c );
    }
    add( vv );
  }
  calc();
}



#endif

