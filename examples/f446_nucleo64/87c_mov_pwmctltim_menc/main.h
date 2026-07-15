#ifndef _MAIN_H
#define _MAIN_H


struct EasingFunInfo {
  EasingFun f;
  float kv;  // max speed koeff, <=1
};

extern const EasingFunInfo part_fun_info[];

#endif

