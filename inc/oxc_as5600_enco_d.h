#ifndef _OXC_AS5600_ENCO_D_H
#define _OXC_AS5600_ENCO_D_H

#include <oxc_as5600.h>

#include <oxc_debug1.h>

#include <oxc_purecaps.h>
#include <oxc_encoder_eng.h>

namespace oxc {

//* EncoderPureCapability interface to AS5600 sensor
class AS5600_Enco_Dev : public EncoderPureCapability
{
  public:
   constexpr explicit AS5600_Enco_Dev( AS5600 &sens_, bool rev_ )
     : sens( sens_ ), eng( AS5600::val2turn-1, rev_ ) {}
   virtual ReturnCode  read()                noexcept override { eng.accept( sens.getAngleNoTurn() ); return rcOk; }
   virtual int32_t  getPos() const           noexcept override { return eng.getPos();    }
   virtual int32_t  getPosRaw() const        noexcept override { return eng.getPosRaw(); }
   virtual int32_t  getDelta() const         noexcept override { return eng.getDelta();  }
   virtual int32_t  getStartPos() const      noexcept override { return eng.getStartPos();  }
   virtual ReturnCode  setStartPos( int32_t pos ) noexcept override { eng.setStartPos( pos ); return rcOk; }

   AS5600* getDev() { return &sens; }

  protected:
   AS5600 &sens;
   EncoderEng eng;
}; // AS5600_Enco_Dev

}; // namespace oxc


#endif

