#ifndef _OXC_PWMCTL_H
#define _OXC_PWMCTL_H


namespace oxc {

//* abstraction for real PWM controllers
class PwmCtl {
  public:
   constexpr explicit PwmCtl( std::size_t n_ch_ ) : n_ch( n_ch_ ) {};
   constexpr std::size_t get_n_ch() const { return n_ch; };
   virtual bool setFreq( uint32_t freq_ ) { return false; };
   virtual uint32_t getFreq() const { return 0; };
   virtual bool setPwmU16( std::size_t ch, uint16_t pwm ) = 0; //* pwm: 0: 65535
   virtual bool setPwm(    std::size_t ch, float pwm )    = 0; //* pwm: 0: 1
  protected:
   std::size_t n_ch;
};

}; // namespace oxc


#endif

