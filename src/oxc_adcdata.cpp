// =====================================================================================
//
//       Filename:  oxc_adcdata.cpp
//
//    Description:  Common data structures and function to store and handle ADC-alike data
//
//        Version:  1.0
//        Created:  2020-03-18 17:17:31
//
//         Author:  Anton Guda (atu), atu@nmetau.edu.ua
//
// =====================================================================================

#include <cmath>

#include <oxc_adcdata.h>

using namespace std;

void StatIntChannel::add( int v )
{
  long long v2 = v * v;
  sum += v;
  sum2 += v2;

  if( v < vmin ) { vmin = v; }
  if( v > vmax ) { vmax = v; }
  ++n;
}

void StatIntChannel::calc()
{
  fsum  = scale * sum;
  fsum2 = scale * scale * sum2;
  mean  = fsum  / n;
  mean2 = fsum2 / n;
  fmin = vmin * scale;
  fmax = vmax * scale;
  if constexpr ( sizeof(xfloat) == sizeof(float) ) {
    sd  = sqrtf( mean2  - mean * mean );
  } else {
    sd  = sqrt(  mean2  - mean * mean );
  }
}


StatIntData::StatIntData( unsigned nch, xfloat a_scale )
  : n_ch( std::min( nch, max_n_ch ) )
{
  for( auto &x : d ) {
    x.set_scale( a_scale );
  }
  reset();
}


void StatIntData::reset()
{
  for( auto &x : d ) {
    x.reset();
  }
  n = 0;
}

void StatIntData::add( const int *v )
{
  for( decltype(+n_ch) j=0; j<n_ch; ++j ) {
    d[j].add( v[j] );
  }
  ++n;
}


void StatIntData::calc()
{
  for( decltype(+n_ch) j=0; j<n_ch; ++j ) {
    d[j].calc();
  }
}

void StatIntData::out_part( OutStream &os, const xfloat StatIntChannel::* pptr, const char *lbl ) const
{
  os << NL << "# " << lbl << "   ";
  for( decltype(+n_ch) j=0; j<n_ch; ++j ) {
    os << ' ' << d[j].*pptr;
  }
}

void StatIntData::out_parts( OutStream &os ) const
{
  os << NL << "# n_real= " << n << " n_ch = " << n_ch;
  for( auto p : structParts ) {
    out_part( os, p.pptr, p.label );
  }
}

