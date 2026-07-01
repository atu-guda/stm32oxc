#ifndef _OXC_ROBOPIN_H
#define _OXC_ROBOPIN_H

#include <oxc_pinbase.h>
#include <oxc_robo_base.h>

// for ReboDevice interface
namespace oxc {

//* RoboDevice interface to Pin
class RoboPin : public RoboDevice
{
  public:
   constexpr explicit RoboPin( PinBase& pin_ ) : RoboDevice( 1 ), pin( pin_ ) {}
   virtual ReturnCode do_commit()  override { pin.write( (bool)(vo) ); return rcOk; }
   virtual ReturnCode do_measure() override { vi = pin.get(); return rcOk; }
   virtual ReturnCode do_accept( size_t ch , int32_t v ) override {
     if( ch == 0 ) {
       vo = v; return rcOk;
     }
     return rcErr;
   }
   virtual exprc_int32_t  get ( size_t ch ) const override {
     if( ch == 0 ) {
       return vi;
     }
     return std::unexpected { rcErr };
   };
  protected:
   PinBase &pin;
   int32_t vi {0};
   int32_t vo {0};
};

}; // namespace oxc

#endif
