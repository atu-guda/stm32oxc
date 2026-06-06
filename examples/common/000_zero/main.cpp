#include <cstring>
#include <charconv>

#include <oxc_auto.h>
#include <oxc_main.h>

using namespace oxc;

USE_DIE_ERROR_HANDLER;



BOARD_DEFINE_LEDS;

int main(void)
{
  // char buf[128] {};
  // // buf[0] = 'x'; buf[0] = '\0';
  // // volatile int v = *(reinterpret_cast<int*>(0x08000000));
  // int v = HAL_GetTick();
  // // i2dec_n( v, buf );
  // auto cres = std::to_chars( buf, buf+std::size(buf), v, 16 );
  // // STD_PROLOG_START;
  //
  // int rc { 0 };
  // if( cres.ec == std::errc() ) {
  //   rc =  buf[0];
  // } else {
  //   rc = 111;
  // }
  //
  // float f = v * 0.1f;
  // cres = std::to_chars( cres.ptr, cres.ptr+50, f );
  // return rc;
  return 0;
}

