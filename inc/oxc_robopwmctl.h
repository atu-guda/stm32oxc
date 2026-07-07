#ifndef _OXC_ROBOPMWCTL_H
#define _OXC_ROBOPMWCTL_H

#include <oxc_pwmctl.h>
#include <oxc_robo_base.h>

namespace oxc {

//* iface to PwmCtl as RoboDevice
class RoboPwmCtl : public RoboDevice {
  public:
   static constexpr size_t max_ch { 32 };
   template<size_t N> // for only string literals as name
     constexpr explicit RoboPwmCtl( const char (&name_)[N], PwmCtl &pwmctl_ ) noexcept
     : RoboDevice( name_ ), pwmctl( pwmctl_ ) {};
   virtual ReturnCode commit()  override { return pwmctl.setPwmsRaw( std::span<uint32_t>( pr, pwmctl.size() ) ); }
   virtual ReturnCode measure() override { return rcOk; }
   virtual ReturnCode initHW()  override { return pwmctl.initHW(); }
   //
   constexpr std::size_t size() const     { return pwmctl.size(); };
   bool setFreq( float freq ) { return pwmctl.setFreq( freq ); };
   float getFreq() const { return pwmctl.getFreq(); };
   bool setPwm(   std::size_t ch, float pwm )   { if( ch >= max_ch) { return false; }; pr[ch] = pwmctl.pwm2raw( pwm );  return true; }
   bool setPulse( std::size_t ch, uint32_t us ) { if( ch >= max_ch) { return false; }; pr[ch] = pwmctl.pulse2raw( us ); return true; }
  protected:
   uint32_t pr[max_ch] {}; // TODO: external storage with good size?
   PwmCtl &pwmctl;
};

}; // namespace oxc


#endif

