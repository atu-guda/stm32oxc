#include <oxc_robo_base.h>

using namespace oxc;

ReturnCode oxc::RoboAssembly::init_all()
{
  for( auto dev : psensors ) {
    auto rc = dev->initHW();
    if( rc.isError() ) {
      return rc;
    }
  }
  for( auto dev : pactuators ) {
    auto rc = dev->initHW();
    if( rc.isError() ) {
      return rc;
    }
  }
  return rcOk;
}

ReturnCode oxc::RoboAssembly::measure_all()
{
  for( auto dev : psensors ) {
    auto rc = dev->measure();
    if( rc.isError() ) {
      return rc;
    }
  }
  return rcOk;
}

ReturnCode oxc::RoboAssembly::commit_all()
{
  for( auto dev : pactuators ) {
    auto rc = dev->commit();
    if( rc.isError() ) {
      return rc;
    }
  }
  return rcOk;
}



