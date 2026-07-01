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
   template<size_t N>
     constexpr explicit RoboPin( const char (&name_)[N], PinBase& pin_ ) : RoboDevice( name_ ), pin( pin_ ) {}
   virtual ReturnCode commit()  override { pin.write( vo ); return rcOk; }
   virtual ReturnCode measure() override { vi = pin.get(); return rcOk; }
   virtual ReturnCode initHW()  override { return pin.initHW(); }
   // only store/read
   void set()           { vo = 1;        }
   void reset()         { vo = 0;        }
   void write( bool v ) { vo = v;        }
   void toggle()        { vo = !vo;      }
   bool get() const     { return vi;     }
  protected:
   PinBase &pin;
   bool vi {false};
   bool vo {false};
};

}; // namespace oxc

#endif
