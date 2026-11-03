#ifndef _MAIN_H
#define _MAIN_H

#include <oxc_easing.h>

struct EasingFunInfo {
  EasingFun f;
  float kv;  // max speed koeff, <=1
};

extern const EasingFunInfo easing_fun_info[];

#endif

