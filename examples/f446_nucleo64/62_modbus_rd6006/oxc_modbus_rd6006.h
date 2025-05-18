#ifndef _OXC_MODBUS_RD6006_H
#define _OXC_MODBUS_RD6006_H

#include <oxc_modbus_rtu_server.h>

class RD6006_Modbus {
  public:
   enum Regs { // only essential
     regSig      = 0,
     regSerial   = 2,
     regFirmWare = 3,
     regTemp     = 5,
     regVset     = 8, // * 100 or 1000 (scale)
     regIset     = 9, // * 1000 or 10000
     regVout     = 0x0A,
     regIout     = 0x0B,
     regWatt     = 0x0D,
     regVin      = 0x0E,
     regLock     = 0x0F,
     regErr      = 0x10, // 1 = OVP, 2 = OCP
     regOn       = 0x12
   };
   enum Vals {
     valSigRD6006  = 60062,
     valSigRD6006P = 60065,
     valErrOVP     = 1,
     valErrOCP     = 2
   };
   RD6006_Modbus( MODBUS_RTU_server &a_mbus ) : mbus( a_mbus ) {};
   void setAddr( uint8_t a_addr ) { addr = a_addr; }
   uint8_t getAddr() const  { return addr; }
   ReturnCode init();
   uint32_t getScale() const { return scale; };
   ReturnCode on()  { return scale ? mbus.writeReg( addr, regOn, 1 ) : rcErr; }
   ReturnCode off() { return scale ? mbus.writeReg( addr, regOn, 0 ) : rcErr; }
   ReturnCode onoff( bool do_on ) { return scale ? mbus.writeReg( addr, regOn, do_on ) : rcErr; }
   bool isOn() { auto v { mbus.readGetReg( addr, regOn) }; return v ? bool(v.value()) : false; }
   ReturnCode lock()   { return mbus.writeReg( addr, regLock, 1 ); }
   ReturnCode unlock() { return mbus.writeReg( addr, regLock, 0 ); }
   uint16_t readErr() { auto v { mbus.readGetReg( addr, regErr ) }; return v.has_value() ? v.value() : 0xFF; }

   ReturnCode setV( uint32_t v_mV )  { return mbus.writeReg( addr, regVset, v_mV * scale / 10 ); }
   ReturnCode setI( uint32_t i_100uA ) { return mbus.writeReg( addr, regIset, i_100uA * scale / 10 ); }
   uint32_t readV_mV();
   uint32_t readI_100uA();
   std::pair<uint32_t,uint32_t> read_VI();

  private:
   MODBUS_RTU_server &mbus;
   uint32_t scale { 0 }; // { 1 or 10, set by init }
   uint8_t addr { 0xFF }; // fake. need setAddr

};


#endif

