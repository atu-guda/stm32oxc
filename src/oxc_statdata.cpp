#include <cmath>

#include <oxc_statdata.h>

using namespace std;

void StatData::Stat1::add( sreal v )
{
  sreal v2 = v * v;

  sreal y = v - kah;
  sreal t = sum + y;
  kah = ( t - sum ) - y;
  sum = t;
  // sum  += v;

  y = v2 - kah2;
  t = sum2 + y;
  kah2 = ( t - sum2 ) - y;
  sum2 = t;

  // sum2 += v2;
  if( v < min ) { min = v; }
  if( v > max ) { max = v; }
  mean  = ( n * mean  + v  ) / (n+1);
  mean2 = ( n * mean2 + v2 ) / (n+1);
  ++n;
}

void StatData::Stat1::calc()
{
  mean  = sum  / n;
  mean2 = sum2 / n;
  if constexpr ( sizeof(sreal) == sizeof(float) ) {
    sd  = sqrtf( mean2  - mean * mean );
  } else {
    sd  = sqrt( mean2  - mean * mean );
  }
}

StatData::StatData( unsigned nch )
  : n_ch( min( nch, max_n_ch ) )
{
  reset();
}

void StatData::reset()
{
  for( auto &x : d ) {
    x.reset();
  }
  n = 0;
}

void StatData::add( const sreal *v )
{
  for( decltype(+n_ch) j=0; j<n_ch; ++j ) {
    d[j].add( v[j] );
  }
  ++n;
}

void StatData::calc()
{
  for( decltype(+n_ch) j=0; j<n_ch; ++j ) {
    d[j].calc();
  }
}

void StatData::out_part( HOST_OSTREAM &os, const sreal Stat1::* pptr, const char *lbl ) const
{
  os << NL "# " << lbl << "   ";
  for( decltype(+n_ch) j=0; j<n_ch; ++j ) {
    os << ' ' << d[j].*pptr;
  }
}

void StatData::out_parts( HOST_OSTREAM &os ) const
{
  os << NL "# n_real= " << n << " n_ch = " << n_ch;
  for( auto p : structParts ) {
    out_part( os, p.pptr, p.label );
  }
}


