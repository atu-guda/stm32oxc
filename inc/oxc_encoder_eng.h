#ifndef _OXC_ENCODER_ENG_H
#define _OXC_ENCODER_ENG_H

// Common engine for encoders

#include <oxc_types.h>

namespace oxc {

class EncoderEng {
 public:
   constexpr EncoderEng( uint32_t max_val = 0xFFFF, bool rev_dir_ = false ) noexcept
         : half_period( (int32_t)(max_val / 2) ),
           signed_period( (int32_t)(max_val + 1) ),
           rev_dir( rev_dir_ ) {};
   int32_t getPos() const noexcept { return pos - start_pos; }
   int32_t getPosRaw() const noexcept { return last_raw_pos; }
   int32_t getStartPos() const noexcept { return start_pos; }
   int32_t getDelta()  const noexcept { return dlt; };
   void    setStartPos( int32_t nsp ) noexcept { start_pos = nsp; pos = last_raw_pos = nsp; };
   void    accept( uint32_t v ) noexcept {
     dlt = (int32_t)v - (int32_t)last_raw_pos;
     // Phase unwrapping
     if( dlt > half_period ) {
       dlt -= signed_period;
     } else if (dlt < -half_period) {
       dlt += signed_period;
     }
     if( rev_dir ) {
       dlt = -dlt;
     }
     pos += dlt;
     last_raw_pos = v;
   };
 protected:
   const int32_t  half_period;
   const int32_t  signed_period;
   int32_t  start_pos { 0 };
   int32_t  pos { 0 };
   int32_t  dlt { 0 };
   uint32_t last_raw_pos { 0 };
   bool     rev_dir;
};


}; // namespace oxc


#endif
