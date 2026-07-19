#include <oxc_robo_base.h>

using namespace oxc;

ReturnCode oxc::RoboAssembly::for_all_till_err( ReturnCode (RoboDevice::*fun)() )
{
  for( auto dev : pdevs ) {
    auto rc = (dev->*fun)();
    if( rc.isError() ) {
      return rc;
    }
  }
  return rcOk;
}


