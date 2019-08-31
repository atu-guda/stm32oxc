#include <cmath>

#include <oxc_statdata.h>

using namespace std;

// ------------------------ StatChannel ---------------------------------------------

void StatChannel::add( sreal v )
{
  sreal v2 = v * v;

  sumKahet( v,  sum,  kah );
  sumKahet( v2, sum2, kah2 );

  if( v < min ) { min = v; }
  if( v > max ) { max = v; }
  ++n;
}

void StatChannel::calc()
{
  mean  = sum  / n;
  mean2 = sum2 / n;
  if constexpr ( sizeof(sreal) == sizeof(float) ) {
    sd  = sqrtf( mean2  - mean * mean );
  } else {
    sd  = sqrt( mean2  - mean * mean );
  }
}

// ------------------------ StatChannelXY ---------------------------------------------

void StatChannelXY::add( sreal v, sreal vy )
{
  StatChannel::add( v );
  sumKahet( v * vy, sum_xy, kah_xy );
}


// ------------------------ StatData --------------------------------------

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

void StatData::out_part( HOST_OSTREAM &os, const sreal StatChannel::* pptr, const char *lbl ) const
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


