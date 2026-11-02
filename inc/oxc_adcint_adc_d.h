#ifndef _OXC_ADCINT_ADC_D_H
#define _OXC_ADCINT_ADC_D_H

#include <oxc_adc.h>
#include <oxc_purecaps.h>


namespace oxc {

class  Adcint_Adc_Dev : public AdcPureCapability {
  public:
   enum { max_ch = 12 }; // may be more, but ...
   constexpr Adcint_Adc_Dev( ADC_Info &adci_, size_t n_ch_, size_t n_bits_, size_t n_aver_ = 1 )
     : adci( adci_ ), n_ch( n_ch_ ), n_bits( n_bits_ ), n_aver( n_aver_ ) { adci.data = hwbuf; }
   ReturnCode init() noexcept { adci.reset_cnt(); return rcOk; }

   virtual ReturnCode read()                     noexcept override;
   virtual int32_t_er getVal( size_t ch ) const  noexcept override {
     if (ch < n_ch) {
       return buf[ch];
     };
     return std::unexpected( rcErr );
   };
   virtual size_t     getNCh() const             noexcept override { return n_ch;   }
   virtual size_t     getNBits() const           noexcept override { return n_bits; }
   void set_n_aver( size_t n_aver_ ) { n_aver = n_aver_; }
   size_t get_n_aver() const { return n_aver; }
  protected:
   ADC_Info &adci;
   size_t n_ch;
   size_t n_bits;
   size_t n_aver;
   uint16_t hwbuf[max_ch];
   uint32_t buf[max_ch];

};



}; // namespace oxc


#endif

