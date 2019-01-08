#ifndef _OXC_STATDATA_H
#define _OXC_STATDATA_H

#include <vector>

#include <oxc_devio.h>
#include <oxc_floatfun.h>


struct StatData {
  struct Stat1 {
    double min, max, mean, sum, sum2, sd;
    Stat1() : min( 1e300) , max(-1e300), mean(0), sum(0), sum2(0), sd(0) {};
  };
  std::vector<Stat1> d;
  unsigned n = 0;
  explicit StatData( unsigned nch );
  void add( const double *v );
  void reset();
  void calc();
};
OutStream& operator<<( OutStream &os, const StatData &sd );

#endif

