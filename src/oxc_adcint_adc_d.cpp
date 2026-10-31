#include <algorithm>
#include <oxc_adcint_adc_d.h>

// #include <oxc_debug1.h> // TODO: remove after debug

using namespace oxc;
using std::size_t;


ReturnCode oxc::Adcint_Adc_Dev::read() noexcept
{
  // std::fill( begin(data), end(data), 0 );

  const uint32_t t_wait0 = 2; // TODO? ms?
  // const uint32_t n_aver = 1; // TODO: param

  // for( uint32_t av=0; av<n_aver; ++av ) {
    std::fill( std::begin(hwbuf), std::end(hwbuf), 0 );

    uint32_t r = adci.start_DMA_wait( n_ch, 1, t_wait0 );
    if( r != 0 ) {
      return rcErr;
    }
    // for( uint32_t i = 0; i<n_ch; ++i ) {
    //   data[i] += buf[i];
    // }
  // }

  // for( uint32_t i = 0; i<n_ch; ++i ) {
  //   data[i] /= n_aver;
  // }

  return rcOk;
}
