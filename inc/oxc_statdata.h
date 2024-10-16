#ifndef _OXC_STATDATA_H
#define _OXC_STATDATA_H

#include <limits>

#ifdef USE_OXC
  #include <oxc_devio.h>
  #include <oxc_floatfun.h>
  #define HOST_OSTREAM OutStream
#else
  #include <iostream>
  #define HOST_OSTREAM std::ostream
  #define NL "\n"
  using xfloat = double;
#endif

template<typename T>
void sumKahet( T v, T &sum, T &kah )
{
  T y = v - kah;
  T t = sum + y;
  kah = ( t - sum ) - y;
  sum = t;
}

struct StatChannel {
  xfloat min = std::numeric_limits<xfloat>::max(),
        max = std::numeric_limits<xfloat>::min(),
        sum = 0, sum2 = 0, mean = 0, mean2 = 0, sd = 0, kah = 0, kah2 = 0;
  unsigned n = 0;
  StatChannel() = default;
  void reset() {
    min = std::numeric_limits<xfloat>::max();
    max = std::numeric_limits<xfloat>::min();
    sum = sum2 = mean = mean2 = sd = kah =  kah2 = 0 ;
    n = 0;
  }
  void add( xfloat v );
  void calc();
};

struct StatChannelXY : public StatChannel {
  xfloat sum_xy = 0, kah_xy = 0;
  StatChannelXY() = default;
  void reset() {
    StatChannel::reset(); sum_xy = kah_xy = 0;
  }
  void add( xfloat v, xfloat vy );
};

struct RegreResults {
  xfloat a, b, r;
  enum ErrorType { noError = 0, diffSize, smallSize, smallDenom, smallDz };
  ErrorType err;
};

bool regre( const StatChannelXY &x, const StatChannel &y, RegreResults &r );

struct StatData {
  static const constexpr unsigned max_n_ch = 8;

  struct StructPart {
    const xfloat StatChannel::* pptr;
    const char* const label;
  };

  StatChannel d[max_n_ch];
  unsigned n = 0;
  unsigned n_ch = 0;

  //
  explicit StatData( unsigned nch );
  auto getNch() const { return n_ch; }
  void add( const xfloat *v );
  void reset();
  void calc();
  void out_part( HOST_OSTREAM &os, const xfloat StatChannel::* pptr, const char *lbl ) const;
  void out_parts( HOST_OSTREAM &os ) const;
  //
  static const constexpr StructPart structParts[] = {
    { &StatChannel::mean,  "mean " },
    { &StatChannel::mean2, "mean2" },
    { &StatChannel::min,   "min  " },
    { &StatChannel::max,   "max  " },
    // { &StatChannel::sum,   "sum  " },
    // { &StatChannel::sum2,  "sum2 " },
    { &StatChannel::sd,    "sd   " }
  };
};

inline HOST_OSTREAM& operator<<( HOST_OSTREAM &os, const StatData &sd ) { sd.out_parts( os );  return os; } ;

#endif

