#ifndef _OXC_ROBO_BASE_H
#define _OXC_ROBO_BASE_H

//* base definitions for robo parts in the oxc library

#include <oxc_types.h>

using std::size_t;
using std::int32_t;

namespace oxc {

//* base abstract class for interface to hardware devices
class RoboDevice {
  public:
   constexpr explicit RoboDevice( size_t n_ch_ ) noexcept : n_ch( n_ch_ ) {};
   RoboDevice( const RoboDevice &rhs ) = delete;
   virtual ~RoboDevice()  = default;
   ReturnCode getStatus() const { return last_status; }
   // reInit()?
   ReturnCode measure()  { return (last_status = do_measure()); }
   ReturnCode commit()   { return (last_status = do_commit()); }
   ReturnCode accept( size_t ch, int32_t v ) { return (last_status = do_accept( ch, v )); }
   virtual exprc_int32_t  get ( size_t ch ) const = 0;
  protected:
   virtual ReturnCode do_measure() = 0;
   virtual ReturnCode do_commit()  = 0;
   virtual ReturnCode do_accept( size_t ch, int32_t v ) = 0;

   size_t n_ch;
   ReturnCode last_status { rcErr };
};

class RoboDeviceOut : public RoboDevice {
  public:
   constexpr explicit RoboDeviceOut( size_t n_ch_ ) noexcept : RoboDevice( n_ch_ ) {};
   RoboDeviceOut( const RoboDeviceOut &rhs ) = delete;
   virtual exprc_int32_t  get ( size_t ch ) const override { return std::unexpected( rcErr ); };
  protected:
   virtual ReturnCode do_measure() override { return rcErr; }
};

class RoboDeviceIn : public RoboDevice {
  public:
   constexpr explicit RoboDeviceIn( size_t n_ch_ ) noexcept : RoboDevice( n_ch_ ) {};
   RoboDeviceIn( const RoboDeviceIn &rhs ) = delete;
  protected:
   virtual ReturnCode do_commit()  override { return rcErr; }
   virtual ReturnCode do_accept( size_t ch, int32_t v ) override { return rcErr; }
};



}; // namespace oxc


#endif

