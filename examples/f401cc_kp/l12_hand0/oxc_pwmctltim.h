#ifndef _OXC_PWMCTLTIM_H
#define _OXC_PWMCTLTIM_H

#include <oxc_tim.h>
#include <oxc_pwmctl.h>

namespace oxc {

//* PWM controller based on STM32 timers with pwm capability
class PwmCtlTim : public PwmCtl {
  public:
   constexpr explicit PwmCtlTim( TIM_TypeDef *tim_, std::size_t n_ch_ ) : PwmCtl( n_ch_ ), tim(tim_) {};
   virtual bool setFreq( uint32_t freq_ ) override;
   virtual uint32_t getFreq() const override;
   virtual bool setPwmU16( std::size_t ch, uint16_t pwm ) override;
   virtual bool setPwm(    std::size_t ch, float pwm )    override;
  protected:
   TIM_TypeDef *tim;

};

}; // namespace oxc


#endif

