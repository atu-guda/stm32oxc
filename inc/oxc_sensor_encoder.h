#ifndef _OXC_SENSOR_ENCODER_H
#define _OXC_SENSOR_ENCODER_H


#include <oxc_floatfun.h>
#include <oxc_robo_base.h>



namespace oxc {

//* abstract class to encoder counter access
class EncoderProxy {
  public:
   virtual exprc_uint32_t get()             = 0;
   virtual ReturnCode     set( uint32_t v ) = 0;
  protected:
};

//* class to encoder counter access as casted pointer to real storage
class EncoderProxyAddr : public EncoderProxy {
  public:
   explicit constexpr EncoderProxyAddr( intptr_t pvi_, bool rw_ = false ) noexcept: pvi ( pvi_ ), rw( rw_ ) {};
   exprc_uint32_t get()             override { return *(reinterpret_cast<uint32_t*>(pvi)); }
   ReturnCode     set( uint32_t v ) override { if( rw ) { *(reinterpret_cast<uint32_t*>(pvi)) = v; }; return rcOk; };
  protected:
   intptr_t pvi;
   bool rw;
};

// TODO: add base class like SensorAS5600Base

// TODO: separate wrapping to own class.

class SensorEncoder : public RoboSensor {
  public:
   template<size_t N>
     SensorEncoder( const char (&name_)[N], EncoderProxy &enc_, uint32_t ppr_, bool rev_dir_ = false, uint32_t max_val_ = 0xFFFF )
         : RoboSensor( name_, 1 ), enc( enc_ ), ppr( ppr_ ),  max_val( max_val_ ),
           half_period( (int32_t)(max_val / 2) ),
           signed_period( (int32_t)(max_val + 1) ),
           rev_dir( rev_dir_ ) {};
   virtual ReturnCode initHW()  override { sta = rcOk; return rcOk; } // not here
   virtual ReturnCode measure() override;
   virtual void setVal( size_t ch, int32_t nv ) override { start_pos = v - nv; };
   virtual int32_t get( size_t ch ) override { return v - start_pos; }
   virtual int32_t getScale( size_t ch ) override { return ppr; }
   uint32_t get_raw_v() const { return last_raw_v; };
   int32_t  get_dlt()   const { return dlt; };
  protected:
   EncoderProxy &enc;
   const uint32_t ppr;           //* pulses per revolution
   const uint32_t max_val;       //* maximum value from HW counter
   const int32_t  half_period;
   const int32_t  signed_period;
   int32_t  start_pos { 0 };
   int32_t  v { 0 };
   int32_t  dlt { 0 };
   uint32_t last_raw_v { 0 };
   bool     rev_dir;

};





} // namespace oxc

#endif // _OXC_SENSOR_ENCODER_H


