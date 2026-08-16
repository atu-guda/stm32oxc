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


class ValFiTrans1x1 {
  public:
   virtual int32_t    toInner( float v ) const noexcept = 0;
   virtual float fromInner( int32_t iv ) const noexcept = 0;
};

class ValFiTrans1xN {
  public:
   virtual bool  toInner( float v, int32_t *ivs ) const noexcept = 0;
   virtual float fromInner( const int32_t *ivs )  const noexcept = 0;
};

class ZeroValFiTrans : public ValFiTrans1x1 {
  public:
   virtual int32_t toInner(  float v   ) const noexcept override { return 0; }
   virtual float fromInner( int32_t iv ) const noexcept override { return 0; }
};

inline static ZeroValFiTrans globalZeroValFiTrans;

class UnityValFiTrans : public ValFiTrans1x1 {
  public:
   virtual int32_t toInner(  float v   ) const noexcept override { return  (int32_t)v;  }
   virtual float fromInner( int32_t iv ) const noexcept override { return  iv; }
};

inline static UnityValFiTrans globalUnityValFiTrans;

class LinearValFiTrans : public ValFiTrans1x1 {
  public:
   explicit constexpr LinearValFiTrans( float a_, float b_ = 0 ) noexcept : // a, b - use in toInner
     a( not_small( a_ )), b( b_ ), ra( 1.0f/a ) {};
   virtual int32_t toInner(    float v ) const noexcept override { return (int32_t)(a*v + b );  }
   virtual float fromInner( int32_t iv ) const noexcept override { return ( iv - b ) * ra;      }
   float a, b, ra;
   static constexpr float not_small( float aa ) { return std::fabsf(aa) > 1e-9f ? aa : 1.0f; }
};

// like LinearValFiTrans, but with correct rounding
class LinRoundValFiTrans : public LinearValFiTrans {
  public:
   explicit constexpr LinRoundValFiTrans( float a_, float b_ = 0 ) noexcept :
     LinearValFiTrans( a_, b_ ) {};
   virtual int32_t toInner(    float v ) const noexcept override { return std::lroundf( a*v + b );  }

};

class LinScaledValFiTrans : public LinearValFiTrans {
  public:
   explicit constexpr LinScaledValFiTrans( float a_, float b_ = 0 ) noexcept :
     LinearValFiTrans( a_, b_ ) {};
   virtual int32_t toInner(    float v ) const noexcept override { return std::lroundf( *pscale * ( a*v + b ) );  }
   void setScaleVar( const int32_t* pscale_ ) noexcept { pscale = pscale_; }
  protected:
   int32_t scale_inner { 1 };
   const int32_t* pscale { & scale_inner };
};

// -------------- More transforms: FF, FI, IF, FI - one-side only ------------------------
// TODO: chain here?

class TransFF {
  public:
   // virtual ~TransFF() = default; // ??? no death by default
   virtual float f( float v ) const noexcept = 0;
};

class TransFI {
  public:
   // virtual ~TransFI() = default; // ???
   virtual int32_t f( float v ) const noexcept = 0;
};

class TransIF {
  public:
   // virtual ~TransIF() = default; // ???
   virtual float f( int32_t v ) const noexcept = 0;
};

class TransII {
  public:
   // virtual ~TransII() = default; // ???
   virtual int32_t f( int32_t v ) const noexcept = 0;
};

// ---------- Trans** basic realizations

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




// ----------------------------------------------------------------------------------------

//* Abstract classes to realize common capabilities + mix to real HW classes

//* pack of digital I/O channels
class IoCapability {
  public:
   explicit constexpr IoCapability( size_t sz_, size_t szF_, size_t bitsz_ ) noexcept :
     sz( sz_ ), szF( szF_ ), bitsz( bitsz_ ) {};
   virtual ~IoCapability() = default; // really unneeded now
   // main interface
   virtual ReturnCode setVal( size_t ch, int32_t v ) noexcept = 0;
   virtual int32_t_er getVal( size_t ch )            noexcept = 0;
   virtual ReturnCode setValF( size_t ch, float v )  noexcept = 0;
   virtual float_er   getValF( size_t ch )           noexcept = 0;
   constexpr size_t size()     const noexcept { return sz;    }
   constexpr size_t sizeF()    const noexcept { return szF;    }
   constexpr size_t bitsize()  const noexcept { return bitsz; }
   //* just convenience functions
   ReturnCode setValF_common( const ValFiTrans1x1 &tr, size_t ch, float v )  noexcept // not so common - lroundf
     { return setVal( ch, tr.toInner( v ) ); }
   float_er   getValF_common( const ValFiTrans1x1 &tr, size_t ch )           noexcept
   {
     auto t = getVal( ch );
     if( !t ) {
       return t;
     }
     return tr.fromInner( t.value() );
   }

  protected:
   const size_t sz;     //* number of integer interface channels
   const size_t szF;    //* number of floating interface channels
   const size_t bitsz;  //* integer channel bitsize - why here? -many different usages?
};



class PinsPureCapability {
  public:
   enum { n_ch_int = 2, n_ch_float = 2, ch_out = 0, ch_in = 1, ch_out_bit = 1, ch_in_bit = 2 };
   virtual int32_t_er read()             noexcept = 0;
   virtual void write(     int32_t v   ) noexcept = 0;
   virtual void set(       int32_t v   ) noexcept = 0;
   virtual void reset(     int32_t v   ) noexcept = 0;
   virtual void toggle(    int32_t v   ) noexcept = 0;
   virtual void setbit(    int32_t pos ) noexcept = 0;
   virtual void resetbit(  int32_t pos ) noexcept = 0;
   virtual void togglebit( int32_t pos ) noexcept = 0;
};


//* pack of pins.
// channels: [0] - set, [1] - get, floats: just copy for now
class PinsCapability : public IoCapability, public PinsPureCapability {
  public:
   explicit constexpr PinsCapability( size_t bitsz_,
       const ValFiTrans1x1 &tr_ = globalUnityValFiTrans ) noexcept :
     IoCapability( n_ch_int, n_ch_float, bitsz_ ), tr( tr_ )  {};
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return setValF_common( tr, ch, v ); }
   virtual float_er   getValF( size_t ch )           noexcept override { return getValF_common( tr, ch ); }
  protected:
   const ValFiTrans1x1 &tr;
};


class PinPureCapability {
  public:
   enum { n_ch_int = 2, n_ch_float = 2, ch_out = 0, ch_in = 1, ch_out_bit = 1, ch_in_bit = 2 };
   virtual int32_t_er read()    noexcept  = 0;
   virtual void write( bool v ) noexcept  = 0;
   virtual void set()           noexcept  = 0;
   virtual void reset()         noexcept  = 0;
   virtual void toggle()        noexcept  = 0;
};


//* Single pin.
// channels: [0] - set, [1] - get
class PinCapability : public IoCapability, public PinPureCapability {
  public:
   constexpr PinCapability( const ValFiTrans1x1 &tr_ = globalUnityValFiTrans ) noexcept :
     IoCapability( n_ch_int, n_ch_float, 1 ), tr( tr_ )  {};
   virtual ReturnCode setValF( size_t ch, float v )  noexcept override { return setValF_common( tr, ch, v ); }
   virtual float_er   getValF( size_t ch )           noexcept override { return getValF_common( tr, ch ); }
  protected:
   const ValFiTrans1x1 &tr;
};



//* frequiency in in Hz, duty: [0:1]
class PwmPureCapability {
  public:
   virtual ReturnCode setDuty( size_t ch, float duty ) noexcept = 0;
   virtual ReturnCode setFreq( float freq )            noexcept = 0;
   virtual float getFreq() const                       noexcept = 0;
};


// channels: 0..sz-1 - duty, sz..sz+n_cfg_ch - freq config
class PwmCapability : public IoCapability, public PwmPureCapability { // + PinsPureCapability?
  public:
   enum { n_cfg_ch = 4 };
    explicit constexpr PwmCapability( size_t sz_, size_t bitsz_,
        const ValFiTrans1xN &tr_f_, LinScaledValFiTrans &tr_d_ ) noexcept
     : IoCapability( sz_ + n_cfg_ch, sz_ + 1, bitsz_ ),
       tr_f( tr_f_ ), tr_d( tr_d_ ) {};
  protected:
   const ValFiTrans1xN &tr_f; // transformation for frequiency
   LinScaledValFiTrans &tr_d; // transformation for duty
};



class EncoderPureCapability {
  public:
};


class EncoderCapability : public IoCapability, public EncoderPureCapability {
  public:
   explicit constexpr EncoderCapability( size_t bitsz_, int32_t scale_ ) noexcept
     : IoCapability( 1, bitsz_, scale_ ) {};
  protected:
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

