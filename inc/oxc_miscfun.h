#ifndef _OXC_MISCFUN_H
#define _OXC_MISCFUN_H

#include <stdint.h>

#include <oxc_base.h>

#ifdef __cplusplus
 extern "C" {
#endif

/** return position of first setted bit LSB=0, or FF if 0 */
uint8_t numFirstBit( uint32_t a );

extern const char hex_digits[];
extern const char dec_digits[];

// 64/log_2[10] \approx 20
#define INT_STR_SZ_DEC 24
#define INT_STR_SZ_HEX 20

// converts char to hex represenration (2 digits+EOL, store to s)
char* char2hex( char c, char *s );
// converts uint32 to hex represenration (8(64=16) digits+EOL, store to s)
char* word2hex( uint32_t d,  char *s );
// 64/log_2[10] \approx 20
#define INT_STR_SZ 24
// if s == 0 returns ptr to inner static buffer
char* i2dec( int n, char *s );

#ifdef __cplusplus
}
#endif

inline int sign( int x );
inline int sign( int x ) { return (x>0) ? 1 : ( (x<0) ? -1: 0 ) ; }

inline int imin( int a, int b  ) { return (a<b) ? a : b; }
inline int imax( int a, int b  ) { return (a>b) ? a : b; }

// converts given arg to int, with check and limits
bool arg2long( int narg, int argc, const char * const * argv, long *v,
               long vmin= INT32_MIN, int vmax = INT32_MAX );
// the same with default value
long arg2long_d( int narg, int argc, const char * const * argv, long def,
                 long vmin = INT32_MIN, long vmax = INT32_MAX );

 // swap bytes in 16-bits
inline uint16_t rev16( uint16_t v ) { return (uint16_t)__REV16( v );}
void rev16( uint16_t *v, int n );

inline uint8_t uint8_to_bcd( uint8_t v ) {
  return ( v % 10 ) + (( v / 10 ) << 4);
}

inline uint8_t bcd_to_uint8( uint8_t bcd ) {
  return ( bcd & 0x0F ) + 10 * ( bcd >> 4 );
}

void bcd_to_char2( uint8_t bcd, char *s );

// converts decimal to 3-byte string w/o null termnation
void u2_3dig( unsigned n, char *s );

#endif

// vim: path=.,/usr/share/stm32cube/inc
