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
     regCC       = 0x11,
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
   ReturnCode on()  { return scale ? mbus.writeReg_ntry( addr, regOn, 1 ) : rcErr; }
   ReturnCode off() { return scale ? mbus.writeReg_ntry( addr, regOn, 0 ) : rcErr; }
   ReturnCode onoff( bool do_on ) { return scale ? mbus.writeReg_ntry( addr, regOn, do_on ) : rcErr; }
   bool isOn() { auto v { mbus.readGetReg_ntry( addr, regOn) }; return v ? bool(v.value()) : false; }
   ReturnCode lock()   { return mbus.writeReg_ntry( addr, regLock, 1 ); }
   ReturnCode unlock() { return mbus.writeReg_ntry( addr, regLock, 0 ); }
   // read - with read, get - by readed before
   uint16_t readErr() { auto v { mbus.readGetReg_ntry( addr, regErr ) }; return v.has_value() ? v.value() : 0xFF; }
   uint16_t readCC()  { auto v { mbus.readGetReg_ntry( addr, regCC )  }; return v.has_value() ? v.value() : 0xFF; }
   ReturnCode readMain() { return mbus.readRegs_ntry( addr, 0, 0x14 ); }

   ReturnCode setV( uint32_t v_mV )  { return mbus.writeReg_ntry( addr, regVset, v_mV * scale / 10 ); }
   ReturnCode setI( uint32_t i_100uA ) { return mbus.writeReg_ntry( addr, regIset, i_100uA * scale / 10 ); }
   uint32_t readV_mV();
   uint32_t readI_100uA();
   uint32_t getV_mV() const    { return mbus.getReg( regVout ) * 10 / scale; }
   uint32_t getI_100uA() const { return mbus.getReg( regIout ) * 10 / scale; }
   uint16_t get_CC()  const { return mbus.getReg( regCC  ); }
   uint16_t get_Err() const { return mbus.getReg( regErr ); }
   std::pair<uint32_t,uint32_t> read_VI();

  private:
   MODBUS_RTU_server &mbus;
   uint32_t scale { 0 }; // { 1 or 10, set by init }
   uint8_t addr { 0xFF }; // fake. need setAddr

};


#endif

