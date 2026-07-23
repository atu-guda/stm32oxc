#include <oxc_sensor_as5600.h>

// using namespace oxc;

oxc::exprc_uint32_t oxc::EncoderProxyAS5600::get()
{
  auto vo = dev.getAngleNoTurn_o();
  if( vo ) {
    return vo.value();
  }
  return std::unexpected { rcErr };
}

ReturnCode     oxc::EncoderProxyAS5600::set( uint32_t v )
{
  return dev.setStartPos( v ) ? rcOk : rcErr;
};



ReturnCode oxc::SensorAS5600::initHW()
{
  auto dev = prox.getDev();
  dev.setCfg( AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off );
  sta = dev.isMagnetDetected() ? rcOk : ReturnCode{ ReturnCode::rcnErr, 2 }; // not detected
  return sta;
}


