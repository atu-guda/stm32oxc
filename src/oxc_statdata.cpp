#include <cmath>

#include <oxc_statdata.h>

using namespace std;

StatData::StatData( unsigned nch )
{
  d.assign( nch, Stat1() );
}

void StatData::reset()
{
  d.assign( d.size(), Stat1() );
  n = 0;
}

void StatData::add( const double *v )
{
  for( unsigned j=0; j<d.size(); ++j ) {
    double cv = v[j];
    if( cv < d[j].min ) { d[j].min = cv; }
    if( cv > d[j].max ) { d[j].max = cv; }
    d[j].sum  += cv;
    d[j].sum2 += cv * cv;
  }
  ++n;
}

void StatData::calc()
{
  for( auto &t : d ) {
    t.mean = t.sum / n;
    t.sd = sqrt(  t.sum2 * n - t.sum * t.sum ) / n;
  }
}

OutStream& operator<<( OutStream &os, const StatData &sd )
{
  auto n_ch = sd.d.size();
  using n_t = decltype( n_ch );
  os << NL "# n_real= " << sd.n << " n_ch = " << n_ch;
  os << NL "# mean   ";
  for( n_t j=0; j<n_ch; ++j ) {
    os << ' ' << sd.d[j].mean;
  }
  os << NL "# min    ";
  for( n_t j=0; j<n_ch; ++j ) {
    os << ' ' << sd.d[j].min;
  }
  os << NL "# max    ";
  for( n_t j=0; j<n_ch; ++j ) {
    os << ' ' << sd.d[j].max;
  }
  os << NL "# sum    ";
  for( n_t j=0; j<n_ch; ++j ) {
    os << ' ' << sd.d[j].sum;
  }
  os << NL "# sum2   ";
  for( n_t j=0; j<n_ch; ++j ) {
    os << ' ' << sd.d[j].sum2;
  }
  os << NL "# sd     ";
  for( n_t j=0; j<n_ch; ++j ) {
    os << ' ' << sd.d[j].sd;
  }
  return os;
}


