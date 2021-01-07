#ifndef _OXC_PICOC_REGHELPERS_H
#define _OXC_PICOC_REGHELPERS_H

#define PICOC_FUN_ARGS  struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs
#define PICOC_C_FUN void fun( PICOC_FUN_ARGS )

#define ARG_0_INT (Param[0]->Val->Integer)
#define ARG_1_INT (Param[1]->Val->Integer)
#define ARG_2_INT (Param[2]->Val->Integer)
#define ARG_3_INT (Param[3]->Val->Integer)
#define ARG_4_INT (Param[4]->Val->Integer)
#define ARG_5_INT (Param[5]->Val->Integer)

#define ARG_0_FP  (Param[0]->Val->FP)
#define ARG_1_FP  (Param[1]->Val->FP)
#define ARG_2_FP  (Param[2]->Val->FP)
#define ARG_3_FP  (Param[3]->Val->FP)
#define ARG_4_FP  (Param[4]->Val->FP)
#define ARG_5_FP  (Param[5]->Val->FP)

#define ARG_0_PTR (Param[0]->Val->Pointer)
#define ARG_1_PTR (Param[1]->Val->Pointer)
#define ARG_2_PTR (Param[2]->Val->Pointer)
#define ARG_3_PTR (Param[3]->Val->Pointer)
#define ARG_4_PTR (Param[4]->Val->Pointer)
#define ARG_5_PTR (Param[5]->Val->Pointer)

#define RV_INT    (ReturnValue->Val->Integer)
#define RV_FP     (ReturnValue->Val->FP)
#define RV_PTR    (ReturnValue->Val->Pointer)

#define FUN_XX_HEAD(nm) \
  void C_ ## nm( PICOC_FUN_ARGS ); \
  void C_ ## nm( PICOC_FUN_ARGS )

#define FUN_D2D(nm) \
  FUN_XX_HEAD(nm) \
  { \
    RV_FP = nm( ARG_0_FP ); \
  }

#define REG_FUN_D2D(nm) \
  { C_ ## nm,         "double " #nm "(double);" }


#define FUN_D2I(nm) \
  FUN_XX_HEAD(nm) \
  { \
    ReturnValue->Val->Integer = nm( ARG_0_FP ); \
  }

#define REG_FUN_D2I(nm) \
  { C_ ## nm,         "int " #nm "(double);" }


#define FUN_I2D(nm) \
  FUN_XX_HEAD(nm) \
  { \
    RV_FP = nm( ARG_0_INT ); \
  }

#define REG_FUN_I2D(nm) \
  { C_ ## nm,         "double " #nm "(int);" }


#define FUN_I2I(nm) \
  FUN_XX_HEAD(nm) \
  { \
    ReturnValue->Val->Integer = nm( ARG_0_INT ); \
  }

#define REG_FUN_I2I(nm) \
  { C_ ## nm,         "int " #nm "(int);" }


#define FUN_DD2D(nm) \
  FUN_XX_HEAD(nm) \
  { \
    RV_FP = nm( ARG_0_FP, ARG_1_FP ); \
  }

#define REG_FUN_DD2D(nm) \
  { C_ ## nm,         "double " #nm "(double,double);" }



#endif

