#include <oxc_sensor_as5600.h>

using namespace oxc;

ReturnCode SensorAS5600::initHW()
{
  dev.setCfg( AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off );
  sta = dev.isMagnetDetected() ? rcOk : ReturnCode{ ReturnCode::rcnErr, 2 }; // not detected
  return sta;
}

ReturnCode SensorAS5600::measure()
{
  auto vo = dev.getAngleN_o();
  if( vo ) {
    v = vo.value();
    return rcOk;
  }
  sta = ReturnCode{ ReturnCode::rcnErr, 3 }; // fail to measure
  return sta;
}


int32_t SensorAS5600::get( size_t /* ch */ )
{
  return v;
}


