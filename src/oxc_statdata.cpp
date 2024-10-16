#include <cmath>

#include <oxc_statdata.h>

using namespace std;

// ------------------------ StatChannel ---------------------------------------------

void StatChannel::add( xfloat v )
{
  xfloat v2 = v * v;

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
  if constexpr ( sizeof(xfloat) == sizeof(float) ) {
    sd  = sqrtf( mean2  - mean * mean );
  } else {
    sd  = sqrt( mean2  - mean * mean );
  }
}

// ------------------------ StatChannelXY ---------------------------------------------

void StatChannelXY::add( xfloat v, xfloat vy )
{
  StatChannel::add( v );
  sumKahet( v * vy, sum_xy, kah_xy );
}

// ---------------------- regre -------------------------------------------

bool regre( const StatChannelXY &x, const StatChannel &y, RegreResults &r )
{
  if( x.n != y.n ) {
    r.err = RegreResults::diffSize;
    return false;
  }
  if( x.n < 2 ) {
    r.err = RegreResults::smallSize;
    return false;
  }

  xfloat dd = x.n * x.sum2 - x.sum * x.sum;

  if( fabsf( dd ) < 1e-6f ) {
    r.err = RegreResults::smallDenom;
    return false;
  }

  const xfloat t1 = x.n * x.sum_xy - x.sum * y.sum;
  r.a = t1 / dd;
  r.b = ( y.sum * x.sum2 - x.sum * x.sum_xy ) / dd;

  const xfloat dz = ( x.n * x.sum2 - x.sum * x.sum ) * ( x.n * y.sum2 - y.sum * y.sum );
  if( dz < (xfloat)1e-6f ) {
    r.err = RegreResults::smallDz;
    return false;
  }
  if constexpr( sizeof(xfloat) == sizeof(float) ) {
    r.r = t1 / sqrtf( dz );
  } else {
    r.r = t1 / sqrt( dz );
  }
  r.err = RegreResults::noError;

  return true;
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

void StatData::add( const xfloat *v )
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

void StatData::out_part( HOST_OSTREAM &os, const xfloat StatChannel::* pptr, const char *lbl ) const
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


