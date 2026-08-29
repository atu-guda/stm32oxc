#ifndef _OXC_CAPABILITIES_H
#define _OXC_CAPABILITIES_H

#include <cmath>
#include <climits>

#include <oxc_purecaps.h>



namespace oxc {

class IoCapability;


// --------------  Transforms: FF, FI, IF, FI - one-side only ------------------------
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
   virtual ~IoCapability() = default; // really unneeded now
   // main interface
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept = 0;
   virtual int32_t_er getVal( size_t ch )            noexcept = 0;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept = 0;
   virtual float_er   getValF( size_t ch )           noexcept = 0;
  protected:
};


//* pack of pins.
class PinsCapability : public IoCapability {
  public:
   enum {
     ch_read = 0, ch_write = 1, ch_set = 2, ch_reset = 3, ch_toggle = 4,
     ch_setbit = 5, ch_resetbit = 6, ch_togglebit = 7, n_ch_int, n_ch_float = 0
   };
   explicit constexpr PinsCapability( PinsPureCapability &pins_ ) noexcept : pins( pins_ ) {};
   PinsCapability( const PinsCapability &r ) = delete;
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch )            noexcept override;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return rcErr; }
   virtual float_er   getValF( size_t ch )           noexcept override { return std::unexpected(rcErr); }
  protected:
   PinsPureCapability &pins;
};


//* Single pin.
class PinCapability : public IoCapability {
  public:
   enum {
     ch_read = 0, ch_write = 1, ch_set = 2, ch_reset = 3, ch_toggle = 4,
     n_ch_int, n_ch_float = 0
   };
   constexpr PinCapability( PinPureCapability &pin_ ) noexcept : pin( pin_ ) {};
   PinCapability( const PinCapability &rhs ) = delete;
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept override;
   virtual int32_t_er getVal( size_t ch )            noexcept override;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return rcErr; }
   virtual float_er   getValF( size_t ch )           noexcept override { return std::unexpected(rcErr); }
  protected:
   PinPureCapability &pin;
};



// channels: 0..sz-1 - duty, sz..sz+n_cfg_ch - freq config
class PwmCapability : public IoCapability {
  public:
    explicit constexpr PwmCapability( PwmPureCapability &pwm_, size_t n_pwm_ch_, size_t bitsz_ ) noexcept
     : pwm( pwm_ ), n_pwm_ch( n_pwm_ch_ ) {};
    PwmCapability( const PwmCapability &r ) = delete;
  protected:
    PwmPureCapability &pwm;
    const size_t n_pwm_ch;
};



class EncoderCapability : public IoCapability {
  public:
   explicit constexpr EncoderCapability( EncoderPureCapability &enc_, size_t bitsz_, int32_t scale_ ) noexcept
     : enc( enc_ ), bitsz( bitsz_ ), scale( scale_ ) {};
   EncoderCapability( const EncoderCapability &r ) = delete;
  protected:
   EncoderPureCapability &enc;
   const size_t bitsz;
   const int32_t scale;
};

// ---------------------------- Channels --------------------------------------------------
// ------------- output: first char {F,I} - from source (control), second - to tagget (device)

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

// ------------- input: first char {F,I} - to control (typeof in()), second - from device

class InChFBase {
  public:
   virtual float in() const noexcept = 0;
};

class InChFConst : public InChFBase {
  public:
   InChFConst( float vc_ = 0 ) noexcept : vc(vc_) {};
   virtual float in() const noexcept override { return vc; }
   float vc;
};

class InChFProxy : public InChFBase {
  public:
   constexpr InChFProxy( const InChFBase &prev_, const TransFF &tr_ ) : prev( prev_ ), tr( tr_ ) {}
   virtual float in() const noexcept override { return tr.f( prev.in() ); };
  protected:
   const InChFBase &prev;
   const TransFF &tr;
};


class InChFSum2 : public InChFBase {
  public:
   constexpr InChFSum2( const InChFBase &prev0_, const InChFBase &prev1_, const TransFF &tr_, float a0_ = 1, float a1_ = 1 )
     : prev0( prev0_ ), prev1( prev1_), tr( tr_ ), a0(a0_), a1(a1_) {}
   virtual float in() const noexcept override { return tr.f( prev0.in() + prev1.in() ); };
  protected:
   const InChFBase &prev0;
   const InChFBase &prev1;
   const TransFF &tr;
   float a0, a1;
};

class InChFFun2 : public InChFBase {
  public:
   constexpr InChFFun2( const InChFBase &prev0_, const InChFBase &prev1_, const TransFF &tr_, float(*fun_)(float,float) )
     : prev0( prev0_ ), prev1( prev1_), tr( tr_ ), fun(fun_) {}
   virtual float in() const noexcept override { return tr.f( fun( prev0.in(), prev1.in() ) ); };
  protected:
   const InChFBase &prev0;
   const InChFBase &prev1;
   const TransFF &tr;
   float (*fun)(float,float);
};



class InChF : public InChFBase {
  public:
   constexpr InChF( IoCapability &cap_, size_t ch_ ) : cap( cap_ ), ch( ch_ ) {}
   virtual float in() const noexcept = 0;
  protected:
   IoCapability &cap;
   const size_t ch;
};

class InChFF : public InChF {
  public:
   constexpr InChFF( IoCapability &cap_, size_t ch_, TransFF &tr_ ) : InChF( cap_, ch_ ), tr( tr_ ) {}
   virtual float in() const noexcept override { return tr.f( cap.getValF( ch ).value_or( NAN ) ); }
  protected:
   const TransFF &tr;
};

class InChFI : public InChF {
  public:
   constexpr InChFI( IoCapability &cap_, size_t ch_, TransIF &tr_ ) : InChF( cap_, ch_ ), tr( tr_ ) {}
   virtual float in() const noexcept override { return tr.f( cap.getVal( ch ).value_or( -1 ) ); };
  protected:
   const TransIF &tr;
};


class InChIBase {
  public:
   virtual int32_t in() const noexcept = 0;
};

class InChIConst : public InChIBase {
  public:
   InChIConst( int32_t vc_ = 0 ) : vc( vc_ ) {};
   virtual int32_t in() const noexcept override { return vc; };
   int32_t vc;
};


class InChIProxy : public InChIBase {
  public:
   constexpr InChIProxy( const InChIBase &prev_, const TransII &tr_ ) : prev( prev_ ), tr( tr_ ) {}
   virtual int32_t in() const noexcept override { return tr.f( prev.in() ); };
  protected:
   const InChIBase &prev;
   const TransII &tr;
};


class InChISum2 : public InChIBase {
  public:
   constexpr InChISum2( const InChIBase &prev0_, const InChIBase &prev1_, const TransII &tr_, int32_t a0_ = 1, int32_t a1_ = 1 )
     : prev0( prev0_ ), prev1( prev1_), tr( tr_ ), a0(a0_), a1(a1_) {}
   virtual int32_t in() const noexcept override { return tr.f( prev0.in() + prev1.in() ); };
  protected:
   const InChIBase &prev0;
   const InChIBase &prev1;
   const TransII &tr;
   int32_t a0, a1;
};


class InChI : public InChIBase {
  public:
   constexpr InChI( IoCapability &cap_, size_t ch_ ) : cap( cap_ ), ch( ch_ ) {}
   virtual int32_t in() const noexcept = 0;
  protected:
   IoCapability &cap;
   const size_t ch;
};

class InChIF : public InChI {
  public:
   constexpr InChIF( IoCapability &cap_, size_t ch_, TransFI &tr_ ) : InChI( cap_, ch_ ), tr( tr_ ) {}
   virtual int32_t in() const noexcept override { return tr.f( cap.getValF( ch ).value_or( 0 ) ); };
  protected:
   const TransFI &tr;
};

class InChII : public InChI {
  public:
   constexpr InChII( IoCapability &cap_, size_t ch_, TransII &tr_ ) : InChI( cap_, ch_ ), tr( tr_ ) {}
   virtual int32_t in() const noexcept override { return tr.f( cap.getVal( ch ).value_or( 0 ) ); };
  protected:
   const TransII &tr;
};



}; //namespace oxc


#endif

