#ifndef _MOVEINFO_H
#define _MOVEINFO_H

#include <oxc_floatfun.h>

struct MoveInfo {
  using Act_Pfun = ReturnCode (*)( MoveInfo &mi, xfloat a, xfloat *coo );
  static const unsigned max_n_koeffs { 10 };
  unsigned n_coo;
  const xfloat  *pa { nullptr }; // ptr to parameters, stored somewhere
  xfloat k_x[max_n_koeffs]; // coeffs
  xfloat len;
  Act_Pfun step_pfun { nullptr }; // calculate each t
  MoveInfo( unsigned a_n_coo, Act_Pfun pfun );
  bool isGood() const { return pa != nullptr && step_pfun != nullptr ; }
  void zero_arr();
  ReturnCode calc_step( xfloat a, xfloat *coo );
  ReturnCode prep_move_line( const xfloat *prm );
  ReturnCode prep_move_circ( const xfloat *prm );
};

// free-standing functions
ReturnCode step_circ_fun( MoveInfo &mi, xfloat a, xfloat *coo );
ReturnCode step_line_fun( MoveInfo &mi, xfloat a, xfloat *coo );

#endif

