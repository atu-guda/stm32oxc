#ifndef _OXC_ADDR_ENCO_D_H
#define _OXC_ADDR_ENCO_D_H

#include <oxc_purecaps.h>
#include <oxc_encoder_eng.h>


namespace oxc {

//* EncoderPureCapability interface to any address for counter
class Addr_Enco_Dev : public EncoderPureCapability
{
  public:
   constexpr explicit Addr_Enco_Dev( std::uintptr_t addr_, int32_t max_val_, bool rev_ )
     : eng( max_val_, rev_ ), addr( addr_ )  {}
   virtual ReturnCode  read()                noexcept override { eng.accept( *(reinterpret_cast<uint32_t*>(addr)) ); return rcOk;}
   virtual int32_t  getPos() const           noexcept override { return eng.getPos();    }
   virtual int32_t  getPosRaw() const        noexcept override { return eng.getPosRaw(); }
   virtual int32_t  getDelta() const         noexcept override { return eng.getDelta();  }
   virtual ReturnCode  setPos( int32_t pos ) noexcept override { eng.setPos( pos ); return rcOk; }

  protected:
   EncoderEng eng;
   std::uintptr_t addr;
}; // Addr_Enco_Dev

}; // namespace oxc

#endif

