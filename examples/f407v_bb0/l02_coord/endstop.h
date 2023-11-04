#ifndef _ENDSTOP_H
#define _ENDSTOP_H

enum ReturnCode;

//* abstract class to represent common endstop
class EndStop {
  public:
   EndStop() = default;
   virtual ReturnCode initHW() = 0;
   virtual uint16_t read() = 0;
   uint16_t get() const { return v; }
   virtual bool is_minus_stop() const = 0;
   virtual bool is_minus_go()   const = 0;
   virtual bool is_plus_stop()  const = 0;
   virtual bool is_plus_go()    const = 0;
   virtual bool is_any_stop()   const = 0;
   virtual bool is_clear()      const = 0;
   virtual bool is_bad()        const = 0;
   virtual bool is_clear_for_dir( int dir ) const = 0;
   virtual char toChar() const = 0;
  protected:
   uint16_t v {0};
};

#endif

