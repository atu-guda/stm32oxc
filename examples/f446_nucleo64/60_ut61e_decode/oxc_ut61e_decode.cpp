#include <cstring>

#include <oxc_auto.h>

#include <oxc_ut61e_decode.h>

using namespace std;


const UT61E_package::FuncInfo UT61E_package::fi[18] = {
  { "I20"   , "A"   , {  -3,  -3,  -3, -3, -3, -3,  -3, -3 } }  , // 0
  { "diode" , "V"   , {  -3,  -3,  -3, -3, -3, -3,  -3, -3 } }  , // 1
  { "duty"  , "%"   , {  -1,  -1,  -1, -1, -1, -1,  -1, -1 } }  , // 2
  { "R"     , "Ohm" , {  -2,  -1,   0,  1,  2,  3,   4,  5 } }  , // 3 no [7]
  { "T"     , "C"   , {   0,   0,   0,  0,  0,  0,   0,  0 } }  , // 4
  { "cont"  , "Ohm" , {   0,   0,   0,  0,  0,  0,   0,  0 } }  , // 5
  { "Cap"   , "F"   , { -12, -11, -10, -9, -8, -7,  -6, -5 } }  , // 6
  { "?37"   , "X"   , {   0,   0,   0,  0,  0,  0,   0,  0 } }  , // 7
  { "?38"   , "X"   , {   0,   0,   0,  0,  0,  0,   0,  0 } }  , // 8
  { "mA"    , "A"   , {   0,   0,   0,  0,  0,  0,   0,  0 } }  , // 9
  { "?3A"   , "X"   , {   0,   0,   0,  0,  0,  0,   0,  0 } }  , // A
  { "V"     , "V"   , {  -4,  -3,  -2, -1, -5, 99,  99, 99 } }  , // B
  { "?3C"   , "X"   , {   0,   0,   0,  0,  0,  0,   0,  0 } }  , // C
  { "uA"    , "A"   , {  -8,  -7,  -6,  0,  0,  0,   0,  0 } }  , // D
  { "A"     , "A"   , {   0,   0,   0,  0,  0,  0,   0,  0 } }  , // E
  { "mA"    , "A"   , {   0,   0,   0,  0,  0,  0,   0,  0 } }  , // F
  { "freq"  , "Hz"  , {  -3,  -2,  -1,  0,  1,  2,   3,  4 } }  , // 10x = 2 + xxx no[2]
  { "xxx1"  , "X"   , {   0,   0,   0,  0,  0,  0,   0,  0 } }    // ??

};

bool UT61E_package::is_good() const
{
  if( cr != '\x0D' || lf != '\x0A' ) {
    // cerr << "Warn: bad CRLF " << endl;
    return false;
  }

  bool was_bad = false;
  const uint8_t *cp = &(range);
  for( unsigned i=0; i<sizeof(*this)-2; ++i ) {
    char c = *cp;
    if( ( c & 0xF0 ) != 0x30 ) {
      // cerr << "Warn: bad char " << hex << c << dec << " pos " << i << endl;
      was_bad = true;
      break;
    }
    ++cp;
  }
  return  !was_bad;
}


int UT61E_package::func_idx() const
{
  if( func < 0x30 ) {
    return -1;
  }
  uint8_t fn = func - 0x30;

  if( func == 0x32 && ! ( status & uint8_t(StatusBits::Judge) ) ) {
    fn = 16;
  }

  if( fn >= size(fi) ) {
    return -2;
  }
  return fn;
}

const char* UT61E_package::func_name() const
{
  auto idx = func_idx();
  if( idx < 0 ) {
    return "???";
  }
  return fi[idx].fname;
}

const char* UT61E_package::value_name() const
{
  auto idx = func_idx();
  if( idx < 0 ) {
    return "???";
  }
  return fi[idx].vname;
}

int UT61E_package::range_exp() const
{
  auto idx = func_idx();
  if( idx < 0 ) {
    return 99;
  }
  return fi[idx].scales[ range & 0x07 ];
}

int32_t UT61E_package::ival() const
{
  if( status & uint8_t(StatusBits::OL) ) {
    return ol_ival;
  }

  int32_t v = 0;
  for( unsigned i=0; i<size(digit); ++i ) {
    v *= 10;
    v += digit[i] & 0x0F;
  }
  if( status & uint8_t(StatusBits::Sign) ) {
    v = -v;
  }
  return v;
}
//
// xreal UT61E_package::val() const
// {
//   int po = range_exp();
//   xreal vd = ival();
//   if( vd <ol_ival && po < 90 ) {
//     vd *= exp10( po );
//   } else {
//     vd = 9.99e37;
//   }
//   return vd;
// }

void UT61E_package::flagsStr( char *s ) const
{
  if( !s ) {
    return;
  }
  *s = 0;
  if( status & (uint8_t)StatusBits::OL ) {
    strcat( s, "OL " );
  }
  if( status & (uint8_t)StatusBits::Batt ) {
    strcat( s, "Batt " );
  }
  if( status & (uint8_t)StatusBits::Sign ) {
    strcat( s, "Sign " );
  }
  if( status & (uint8_t)StatusBits::Judge ) {
    strcat( s, "Judge " );
  }

  if( opt[0] & 0x01 ) {
    strcat( s, "RMD " );
  }
  if( opt[0] & 0x02 ) {
    strcat( s, "RE " );
  }
  if( opt[0] & 0x04 ) {
    strcat( s, "MIN " );
  }
  if( opt[0] & 0x08 ) {
    strcat( s, "MAX " );
  }

  if( opt[1] & 0x01 ) {
    strcat( s, "?11 " );
  }
  if( opt[1] & 0x02 ) {
    strcat( s, "Pmin " );
  }
  if( opt[1] & 0x04 ) {
    strcat( s, "Pmax " );
  }
  if( opt[1] & 0x08 ) {
    strcat( s, "UL " );
  }

  if( opt[2] & 0x01 ) {
    strcat( s, "VAH7 " );
  }
  if( opt[2] & 0x02 ) {
    strcat( s, "Auto " );
  }
  if( opt[2] & 0x04 ) {
    strcat( s, "AC " );
  }
  if( opt[2] & 0x08 ) {
    strcat( s, "DC " );
  }


  if( opt[3] & 0x01 ) {
    strcat( s, "?31 " );
  }
  if( opt[3] & 0x02 ) {
    strcat( s, "VBAR " );
  }
  if( opt[3] & 0x04 ) {
    strcat( s, "HOLD " );
  }
  if( opt[3] & 0x08 ) {
    strcat( s, "LPF " );
  }

}


