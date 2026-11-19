#ifndef _OXC_MOTORDIR_1V2D_ROBOCTL_H
#define _OXC_MOTORDIR_1V2D_ROBOCTL_H

#include <oxc_robocaps.h>

namespace oxc {

// Like TB6612FNG: pins(LRP): HH* = break, XYH = go, XYL = break, LL* = stop/coast, adjusted by break_low

// PWM-alike controlled: 1 analog/pwm channel, 2 direction pins
class MotorDir_1V2D_RoboCtl : public IoRoboCapability {
  public:
   MotorDir_1V2D_RoboCtl( InChFBase &v_in_, InChIBase  &idle_in_, InChIBase  &break_in_,
       OutChFBase &v_out_, OutChIBase &left_out_, OutChIBase &right_out_, uint32_t id_  )
     : IoRoboCapability( id_ ),
       v_in( v_in_ ), idle_in( idle_in_ ), break_in( break_in_ ),
       v_out( v_out_ ), left_out( left_out_ ), right_out( right_out_ )
   { }

   enum { /* float: */ ch_v_in = 0, ch_v_out = 1, /* int: */ ch_left_out = 0, ch_right_out = 1 };

   virtual ReturnCode setVal(  size_t ch, int32_t v ) noexcept override { return rcErr; };
   virtual int32_t_er getVal(  size_t ch )            noexcept override;
   virtual ReturnCode setValF( size_t ch, float v )   noexcept override { return rcErr; }
   virtual float_er   getValF( size_t ch )            noexcept override;

   void set_break_v0( int32_t break_v0_ ) { break_v0 = break_v0_; }
   void set_break_low( int32_t break_low_ ) { break_low = break_low_; }
   void set_zero_v( float v_ ) { zero_v = v_; };

  protected:
   virtual ReturnCode doInit()    noexcept;
   virtual ReturnCode doMeasure() noexcept;
   virtual ReturnCode doThink()   noexcept;
   virtual ReturnCode doCommit()  noexcept;

   void do_break() noexcept;
   void do_idle()  noexcept;

  protected:
   InChFBase  &v_in;      //* signed normalized velocity input: [-1:1]
   InChIBase  &idle_in;   //* flag: make idle = coast
   InChIBase  &break_in;  //* flag: make break, less prty than idle TODO: rethink?

   OutChFBase &v_out;     //* output velocity-alike one-dir [0:1] control like pwm
   OutChIBase &left_out;  //* pin-alike: rotate left
   OutChIBase &right_out; //* pin-alike: rotate right

   float vi { 0 };
   float vo { 0 };
   int32_t o_left  { 0 };
   int32_t o_right { 0 };

   int32_t break_v0 {0};    //* flag: set "break" output at near zero v, else - coast
   int32_t break_low {0};   //* flag: 1: "break" is done as LL signal, default = HH
   float   zero_v { 1e-5f }; //* if |v| is set lower then this - stop

};


}; // namespace oxc;

#endif

