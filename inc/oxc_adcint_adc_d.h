#ifndef _OXC_ADCINT_ADC_D_H
#define _OXC_ADCINT_ADC_D_H

#include <oxc_adc.h>
#include <oxc_purecaps.h>


namespace oxc {

class  Adcint_Adc_Dev : public AdcPureCapability {
  public:
   enum { max_ch = 12 }; // may be more, but ...
   constexpr Adcint_Adc_Dev( ADC_Info &adci_, size_t n_ch_, size_t n_bits_ )
     : adci( adci_ ), n_ch( n_ch_ ), n_bits( n_bits_ ) {}
   ReturnCode init() noexcept { adci.reset_cnt(); return rcOk; }

   virtual ReturnCode read()                     noexcept override;
   virtual int32_t_er getVal( size_t ch ) const  noexcept override {
     if (ch < n_ch) {
       return hwbuf[ch];
     };
     return std::unexpected( rcErr );
   };
   virtual size_t     getNCh() const             noexcept override { return n_ch;   }
   virtual size_t     getNBits() const           noexcept override { return n_bits; }
  protected:
   ADC_Info &adci;
   size_t n_ch;
   size_t n_bits;
   uint16_t hwbuf[max_ch];

};



}; // namespace oxc


#endif

