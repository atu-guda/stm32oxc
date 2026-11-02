#include <algorithm>
#include <ranges>
#include <oxc_adcint_adc_d.h>

// #include <oxc_debug1.h> // TODO: remove after debug

using namespace oxc;
using std::size_t;


ReturnCode oxc::Adcint_Adc_Dev::read() noexcept
{
  std::ranges::fill( buf, 0 );

  const uint32_t t_wait0 = 2; // TODO? ms?

  for( uint32_t av=0; av<n_aver; ++av ) {
    std::ranges::fill( hwbuf, 0 );

    uint32_t r = adci.start_DMA_wait( n_ch, 1, t_wait0 );
    if( r != 0 ) {
      return rcErr;
    }

    // auto zipped_view = std::views::zip( buf, hwbuf ) | std::views::take( n_ch );
    // for( auto&& [acc, v] : zipped_view ) {
    //     acc += v;
    // }

    for( decltype(+n_ch) i {0}; i<n_ch; ++i ) {
      buf[i] += hwbuf[i];
    }
  }

  const auto div_val = 1.0f / n_aver; // TODO: not always div!
  for( decltype(+n_ch) i {0}; i<n_ch; ++i ) {
    buf[i] *= div_val;
  }

  return rcOk;
}
