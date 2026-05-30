#ifndef _OXC_PINBASE_H
#define _OXC_PINBASE_H


namespace oxc {

//* abstract base for different simple I/O pin actions
//* not all must be real
class PinBase {
  public:
   virtual void set()           = 0;
   virtual void reset()         = 0;
   virtual void write( bool v ) = 0;
   virtual void toggle()        = 0;
   virtual bool get()           = 0;
   virtual bool initHW()        = 0;
};


}; // namespace oxc;


#endif


