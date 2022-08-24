#ifndef _OXC_STEPMOTOR_H
#define _OXC_STEPMOTOR_H

// abstract stepmotor control

// for std::size
#include <iterator>

#include <oxc_base.h>

class StepMotorDriverBase {
  public:
   virtual void set( uint16_t outs ) noexcept = 0;
   virtual void init() noexcept = 0;
};

class StepMotor
{
  public:
   struct MotorMode {
     std::size_t n_steps;
     const uint16_t *steps;
   };
   StepMotor( StepMotorDriverBase &a_drv, std::size_t mode ) noexcept
     : drv( a_drv )
     { setMode( mode ); };
   // void set( uint16_t v ) noexcept { drv.set( v ); }
   void off() noexcept { drv.set( 0 ); }
   void init() noexcept { drv.init(); ph = 0; }
   void setMode( size_t a_mode ) noexcept;
   size_t getMode() const noexcept { return mode; }
   void setExternMode( const uint16_t *st, size_t ns ) noexcept;
   size_t getPhase() const noexcept { return ph; }
   uint16_t getV() const noexcept { return steps[ph]; }
   void setPhase( size_t phase ) noexcept { ph = phase % n_steps; }
   void stepF() { return step(  1 ); }
   void stepB() { return step( -1 ); };
   void step( int v );
  protected:
   StepMotorDriverBase &drv;
   const uint16_t *steps { nullptr };
   size_t n_steps { 0 };
   size_t ph { 0 };
   size_t mode { 0 };
   static constexpr uint16_t half_steps4[] { 1, 3, 2, 6, 4, 12, 8, 9 };
   static constexpr uint16_t full_steps4[] { 1, 2, 4, 8 };
   static constexpr uint16_t half_steps3[] { 1, 3, 2, 6, 4, 5 };
   static constexpr uint16_t full_steps3[] { 1, 2, 4 };
   static constexpr MotorMode m_modes[] = {
     { std::size(full_steps4), full_steps4 },
     { std::size(half_steps4), half_steps4 },
     { std::size(half_steps3), half_steps3 },
     { std::size(half_steps3), full_steps3 }
   };
   static constexpr size_t n_modes = std::size(m_modes);
};


#endif

