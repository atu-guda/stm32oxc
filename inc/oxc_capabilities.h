#ifndef _OXC_CAPABILITIES_H
#define _OXC_CAPABILITIES_H

#include <cmath>
#include <climits>

#include <oxc_types.h>

using std::size_t;
using std::uint32_t;
using std::int32_t;

using oxc::ReturnCode;


namespace oxc {

class IoCapability;


// -------------- More transforms: FF, FI, IF, FI - one-side only ------------------------
// TODO: chain here?

class TransFF {
  public:
   virtual float f( float v ) const noexcept = 0;
};

class TransFI {
  public:
   virtual int32_t f( float v ) const noexcept = 0;
};

class TransIF {
  public:
   virtual float f( int32_t v ) const noexcept = 0;
};

class TransII {
  public:
   virtual int32_t f( int32_t v ) const noexcept = 0;
};

// ---------- Trans** basic realizations

// ----------------- FF

class TransFFUnity : public TransFF {
  public:
   virtual float f( float v ) const noexcept override { return v; }
};

inline TransFFUnity globalTransFFUnity;

class TransFFLin : public TransFF {
  public:
   constexpr TransFFLin ( float a_, float b_ ) : a(a_), b(b_) {}
   virtual float f( float v ) const noexcept override { return a*v + b; }
   float a, b;
};

class TransFFLinLim : public TransFFLin {
  public:
   constexpr TransFFLinLim ( float a_, float b_, float xmin_ = -__FLT_MAX__, float xmax_ = __FLT_MAX__ )
     : TransFFLin( a_, b_ ) , xmin( xmin_ ), xmax( xmax_ ) {}
   virtual float f( float v ) const noexcept override { return std::clamp( a*v + b, xmin, xmax ); }
   float xmin, xmax;
};

// ----------------- FI

class TransFIUnity : public TransFI {
  public:
   virtual int32_t f( float v ) const noexcept override { return (int32_t)( v ); }
};
inline TransFIUnity globalTransFIUnity;

class TransFILin : public TransFI {
  public:
   constexpr TransFILin ( float a_, float b_ ) : a(a_), b(b_) {}
   virtual int32_t f( float v ) const noexcept override { return (int32_t)( a*v + b ); }
   float a, b;
};

class TransFILinRound : public TransFILin {
  public:
   constexpr TransFILinRound ( float a_, float b_ )
     : TransFILin( a_, b_ ) {}
   virtual int32_t f( float v ) const noexcept override { return std::lroundf( (int32_t)(a*v + b) ); }
};


class TransFILinLim : public TransFILin {
  public:
   constexpr TransFILinLim ( float a_, float b_, int32_t xmin_ = INT_MIN, int32_t xmax_ = INT_MAX )
     : TransFILin( a_, b_ ) , xmin( xmin_ ), xmax( xmax_ ) {}
   virtual int32_t f( float v ) const noexcept override { return std::clamp( (int32_t)(a*v + b), xmin, xmax ); }
   int32_t xmin, xmax;
};


// ----------------- IF

class TransIFUnity : public TransIF {
  public:
   virtual float f( int32_t v ) const noexcept override { return (float)( v ); }
};
inline TransIFUnity globalTransIFUnity;


class TransIFLin : public TransIF {
  public:
   constexpr TransIFLin ( float a_, float b_ ) : a(a_), b(b_) {}
   virtual float f( int32_t v ) const noexcept override { return ( a*v + b ); }
   float a, b;
};



// ----------------- II
class TransIIUnity : public TransII {
  public:
   virtual int32_t f( int32_t v ) const noexcept override { return v; }
};
inline TransIIUnity globalTransIIUnity;

class TransIILin : public TransII {
  public:
   constexpr TransIILin ( int32_t an_, int32_t ad_, int32_t b_ ) : an(an_), ad(ad_), b(b_) {}
   virtual int32_t f( int32_t v ) const noexcept override { return v * an / ad + b; }
   int32_t an, ad, b;
};

// ----------------------------------------------------------------------------------------

//* Abstract classes to realize common capabilities + mix to real HW classes

//* pack of digital I/O channels
class IoCapability {
  public:
   explicit constexpr IoCapability( size_t sz_, size_t szF_ ) noexcept :
     sz( sz_ ), szF( szF_ ) {};
   virtual ~IoCapability() = default; // really unneeded now
   // main interface
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept = 0;
   virtual int32_t_er getVal( size_t ch )            noexcept = 0;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept = 0;
   virtual float_er   getValF( size_t ch )           noexcept = 0;
   constexpr size_t size()     const noexcept { return sz;    }
   constexpr size_t sizeF()    const noexcept { return szF;    }

  protected:
   const size_t sz;     //* number of integer interface channels
   const size_t szF;    //* number of floating interface channels
};



class PinsPureCapability {
  public:
   virtual int32_t_er read()             noexcept = 0; // 0 r - index for PinsCapability
   virtual int32_t_er readwr()           noexcept = 0; // 1 r -|
   virtual void write(     int32_t v   ) noexcept = 0; // 1 w -|
   virtual void set(       int32_t v   ) noexcept = 0; // 2 w
   virtual void reset(     int32_t v   ) noexcept = 0; // 3
   virtual void toggle(    int32_t v   ) noexcept = 0; // 4
   virtual void setbit(    int32_t pos ) noexcept = 0; // 5
   virtual void resetbit(  int32_t pos ) noexcept = 0; // 6
   virtual void togglebit( int32_t pos ) noexcept = 0; // 7
};


//* pack of pins.
class PinsCapability : public IoCapability {
  public:
   enum {
     ch_read = 0, ch_write = 1, ch_set = 2, ch_reset = 3, ch_toggle = 4,
     ch_setbit = 5, ch_resetbit = 6, ch_togglebit = 7, n_ch_int, n_ch_float = 0
   };
   explicit constexpr PinsCapability( PinsPureCapability &pins_ ) noexcept :
     IoCapability( n_ch_int, n_ch_float ), pins( pins_ ) {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch )            noexcept override;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return rcErr; }
   virtual float_er   getValF( size_t ch )           noexcept override { return std::unexpected(rcErr); }
  protected:
   PinsPureCapability &pins;
};

//* single pin
class PinPureCapability {
  public:
   virtual int32_t_er read()    noexcept  = 0; // 0 - r
   virtual int32_t_er readwr()  noexcept  = 0; // 1 - r
   virtual void write( bool v ) noexcept  = 0; // 1 - w
   virtual void set()           noexcept  = 0; // 2 - w
   virtual void reset()         noexcept  = 0; // 3 - w
   virtual void toggle()        noexcept  = 0; // 4 - w
};


//* Single pin.
class PinCapability : public IoCapability, public PinPureCapability {
  public:
   enum {
     ch_read = 0, ch_write = 1, ch_set = 2, ch_reset = 3, ch_toggle = 4,
     n_ch_int, n_ch_float = 0
   };
   constexpr PinCapability( PinPureCapability &pin_ ) noexcept :
     IoCapability( n_ch_int, n_ch_float ), pin( pin_ ) {};
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch )            noexcept override;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return rcErr; }
   virtual float_er   getValF( size_t ch )           noexcept override { return std::unexpected(rcErr); }
  protected:
   PinPureCapability &pin;
};



//* frequiency in Hz, duty: [0:1]
class PwmPureCapability {
  public:
   virtual ReturnCode setDuty(  size_t ch, float duty ) noexcept = 0;
   virtual ReturnCode setPulse( size_t ch, float pu_s ) noexcept = 0;
   virtual ReturnCode setFreq( float freq )             noexcept = 0;
   virtual float getFreq() const                        noexcept = 0;
};


// channels: 0..sz-1 - duty, sz..sz+n_cfg_ch - freq config
class PwmCapability : public IoCapability {
  public:
    explicit constexpr PwmCapability( PwmPureCapability &pwm_, size_t n_pwm_ch_, size_t bitsz_ ) noexcept
     : IoCapability( 0, n_pwm_ch_ * 2 + 1 ), pwm( pwm_ ), n_pwm_ch( n_pwm_ch_ ), bitsz( bitsz_ )
       {};
  protected:
    PwmPureCapability &pwm;
    const size_t n_pwm_ch;
    const size_t bitsz;
};



class EncoderPureCapability {
  public:
};


class EncoderCapability : public IoCapability {
  public:
   explicit constexpr EncoderCapability( EncoderPureCapability &enc_, size_t bitsz_, int32_t scale_ ) noexcept
     : IoCapability( 1, 0 ), enc( enc_ ), bitsz( bitsz_ ), scale( scale_ ) {};
  protected:
   EncoderPureCapability &enc;
   const size_t bitsz;
   const int32_t scale;
};

// ---------------------------- Channels --------------------------------------------------


class OutChFBase {
  public:
   // virtual ~OutChFBase() = default;
   virtual void out( float v ) const noexcept = 0;
};

// just ignore
class OutChFNull : public OutChFBase {
  public:
   virtual void out( float /* v */ ) const noexcept override {};
};


class OutChFProxy : public OutChFBase {
  public:
   constexpr OutChFProxy( const OutChFBase &next_, const TransFF &tr_ ) : next( next_ ), tr( tr_ ) {}
   virtual void out( float v ) const noexcept override { next.out( tr.f( v ) ); };
  protected:
   const OutChFBase &next;
   const TransFF &tr;
};


class OutChFSplit2 : public OutChFBase {
  public:
   constexpr OutChFSplit2( const OutChFBase &next0_, const OutChFBase &next1_, const TransFF &tr_ )
     : next0( next0_ ), next1( next1_), tr( tr_ ) {}
   virtual void out( float v ) const noexcept override { next0.out( tr.f( v ) ); next1.out( tr.f( v ) ); };
  protected:
   const OutChFBase &next0;
   const OutChFBase &next1;
   const TransFF &tr;
};



class OutChF : public OutChFBase {
  public:
   constexpr OutChF( IoCapability &cap_, size_t ch_ ) : cap( cap_ ), ch( ch_ ) {}
   virtual void out( float v ) const noexcept = 0;
  protected:
   IoCapability &cap;
   const size_t ch;
};

class OutChFF : public OutChF {
  public:
   constexpr OutChFF( IoCapability &cap_, size_t ch_, TransFF &tr_ ) : OutChF( cap_, ch_ ), tr( tr_ ) {}
   virtual void out( float v ) const noexcept override { cap.setValF( ch, tr.f( v ) ); };
  protected:
   const TransFF &tr;
};

class OutChFI : public OutChF {
  public:
   constexpr OutChFI( IoCapability &cap_, size_t ch_, TransFI &tr_ ) : OutChF( cap_, ch_ ), tr( tr_ ) {}
   virtual void out( float v ) const noexcept override { cap.setVal( ch, tr.f( v ) ); };
  protected:
   const TransFI &tr;
};


class OutChIBase {
  public:
   // virtual ~OutChIBase() = default;
   virtual void out( int32_t v ) const noexcept = 0;
};

class OutChINull : public OutChIBase {
  public:
   virtual void out( int32_t /* v */ ) const noexcept override {};
};


class OutChIProxy : public OutChIBase {
  public:
   constexpr OutChIProxy( const OutChIBase &next_, const TransII &tr_ ) : next( next_ ), tr( tr_ ) {}
   virtual void out( int32_t v ) const noexcept override { next.out( tr.f( v ) ); };
  protected:
   const OutChIBase &next;
   const TransII &tr;
};


class OutChISplit2 : public OutChIBase {
  public:
   constexpr OutChISplit2( const OutChIBase &next0_, const OutChIBase &next1_, const TransII &tr_ )
     : next0( next0_ ), next1( next1_), tr( tr_ ) {}
   virtual void out( int32_t v ) const noexcept override { next0.out( tr.f( v ) ); next1.out( tr.f( v ) ); };
  protected:
   const OutChIBase &next0;
   const OutChIBase &next1;
   const TransII &tr;
};


class OutChI : public OutChIBase {
  public:
   constexpr OutChI( IoCapability &cap_, size_t ch_ ) : cap( cap_ ), ch( ch_ ) {}
   virtual ~OutChI() = default;
   virtual void out( int32_t v ) const noexcept = 0;
  protected:
   IoCapability &cap;
   const size_t ch;
};

class OutChIF : public OutChI {
  public:
   constexpr OutChIF( IoCapability &cap_, size_t ch_, TransIF &tr_ ) : OutChI( cap_, ch_ ), tr( tr_ ) {}
   virtual void out( int32_t v ) const noexcept override { cap.setValF( ch, tr.f( v ) ); };
  protected:
   const TransIF &tr;
};

class OutChII : public OutChI {
  public:
   constexpr OutChII( IoCapability &cap_, size_t ch_, TransII &tr_ ) : OutChI( cap_, ch_ ), tr( tr_ ) {}
   virtual void out( int32_t v ) const noexcept override { cap.setVal( ch, tr.f( v ) ); };
  protected:
   const TransII &tr;
};




}; //namespace oxc


#endif

