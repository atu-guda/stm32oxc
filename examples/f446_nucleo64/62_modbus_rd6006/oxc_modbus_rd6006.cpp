#include <oxc_modbus_rd6006.h>

ReturnCode RD6006_Modbus::init()
{
  scale = 0;
  auto sig { mbus.readGetReg( addr, regSig ) };
  if( !sig ) {
    return rcErr;
  }
  if( sig.value() == valSigRD6006 ) {
    scale = 1;
    return rcOk;
  }
  if( sig.value() == valSigRD6006P ) {
    scale = 10;
    return rcOk;
  }
  return rcErr;
}

uint32_t RD6006_Modbus::readV_mV()
{
  auto v { mbus.readGetReg( addr, regVout ) };
  if( !v || !scale ) {
    return 0;
  }
  return v.value() * 10 / scale;
}

uint32_t RD6006_Modbus::readI_100uA()
{
  auto v { mbus.readGetReg( addr, regIout ) };
  if( !v || !scale ) {
    return 0;
  }
  return v.value() * 10 / scale;
}

std::pair<uint32_t,uint32_t> RD6006_Modbus::read_VI()
{
  if( !scale ) {
    return {0,0};
  }
  mbus.readRegs( addr, 0, 0x14 ); // FAKE to re-read?
  delay_ms( 50 );
  auto rc = mbus.readRegs( addr, 0, 0x14 );
  if( rc != rcOk ) {
    return {0,0};
  }
  uint32_t v = mbus.getReg(regVout) * 10 / scale;
  uint32_t i = mbus.getReg(regIout) * 10 / scale;

  return { v, i };
}

