#pragma once

#include <algorithm>

using EasingFun = float (*)(float); // [0..1] -> [0..1]

inline float easing_lim( float x ) { return std::clamp( x, 0.0f, 1.0f ); };
float easing_one(      float x ); // y = x + lim
float easing_poly2_in( float x );
float easing_poly2_out( float x );
float easing_poly3_io( float x );
float easing_trig_in(  float x );
float easing_trig_out(  float x );
float easing_trig_io(  float x );
float easing_step_01(  float x );

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

