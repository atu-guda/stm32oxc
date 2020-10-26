#ifndef _OXC_PICOC_REGHELPERS_H
#define _OXC_PICOC_REGHELPERS_H

#define FUN_XX_HEAD(nm) \
  void C_ ## nm( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs); \
  void C_ ## nm( struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)

#define FUN_D2D(nm) \
  FUN_XX_HEAD(nm) \
  { \
    ReturnValue->Val->FP = nm( Param[0]->Val->FP ); \
  }

#define REG_FUN_D2D(nm) \
  { C_ ## nm,         "double " #nm "(double);" }


#define FUN_D2I(nm) \
  FUN_XX_HEAD(nm) \
  { \
    ReturnValue->Val->Integer = nm( Param[0]->Val->FP ); \
  }

#define REG_FUN_D2I(nm) \
  { C_ ## nm,         "int " #nm "(double);" }


#define FUN_I2D(nm) \
  FUN_XX_HEAD(nm) \
  { \
    ReturnValue->Val->FP = nm( Param[0]->Val->Integer ); \
  }

#define REG_FUN_I2D(nm) \
  { C_ ## nm,         "double " #nm "(int);" }


#define FUN_I2I(nm) \
  FUN_XX_HEAD(nm) \
  { \
    ReturnValue->Val->Integer = nm( Param[0]->Val->Integer ); \
  }

#define REG_FUN_I2I(nm) \
  { C_ ## nm,         "int " #nm "(int);" }


#define FUN_DD2D(nm) \
  FUN_XX_HEAD(nm) \
  { \
    ReturnValue->Val->FP = nm( Param[0]->Val->FP, Param[1]->Val->FP ); \
  }

#define REG_FUN_DD2D(nm) \
  { C_ ## nm,         "double " #nm "(double,double);" }



#endif

