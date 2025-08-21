#ifndef _OXC_UT61E_DECODE_H
#define _OXC_UT61E_DECODE_H

const unsigned UT61E_PKT_SZ = 14;

// all bytes except crlf is in 0x3X form
struct UT61E_package
{
  struct FuncInfo {
    const char* const fname;
    const char* const vname;
    const int scales[8];
  };
  enum class StatusBits { OL = 1, Batt = 2, Sign = 4, Judge = 8 };
  uint8_t range;
  uint8_t digit[5]; // digits, MS - first
  uint8_t func;
  uint8_t status;
  uint8_t opt[4];
  uint8_t cr;
  uint8_t lf;
  //
  bool is_good() const;
  int func_idx() const;
  const char* func_name() const;
  const char* value_name() const;
  int  range_exp() const;
  int32_t ival() const;
  // xreal val() const;
  void flagsStr( char *s ) const; // not C++ string: for MC

  const static FuncInfo fi[18]; // = 16 + 1freq + res
  static const int ol_ival = 99999999;
} __attribute((packed));

static_assert( sizeof(UT61E_package) == UT61E_PKT_SZ, "Bad UT61E_package size, must be 14" );



#endif

