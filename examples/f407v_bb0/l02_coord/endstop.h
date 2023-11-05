#ifndef _ENDSTOP_H
#define _ENDSTOP_H

enum ReturnCode;

//* abstract class to represent common endstop
class EndStop {
  public:
   enum StopBits { minusBit = 0x01, plusBit = 0x02, extraBit = 0x04, mainBits = 0x03 };
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

//* fake endstop: all good
class EndStopGood : public EndStop {
  public:
   EndStopGood() { v = 0xFF; };
   virtual ReturnCode initHW() override { return rcOk; };
   virtual uint16_t read() override { v = 0xFF; return v; };
   virtual bool is_minus_stop() const override { return false; };
   virtual bool is_minus_go()   const override { return true;  };
   virtual bool is_plus_stop()  const override { return false; };
   virtual bool is_plus_go()    const override { return true;  };
   virtual bool is_any_stop()   const override { return false; };
   virtual bool is_clear()      const override { return true;  };
   virtual bool is_bad()        const override { return false; };
   virtual bool is_clear_for_dir( int dir ) const override { return true; };
   virtual char toChar() const override { return '.'; };
};

//* fake endstop: all bad
class EndStopBad : public EndStop {
  public:
   EndStopBad();
   virtual ReturnCode initHW() override { return rcOk; };
   virtual uint16_t read() override { v = 0; return v; };
   virtual bool is_minus_stop() const override { return true; };
   virtual bool is_minus_go()   const override { return false;  };
   virtual bool is_plus_stop()  const override { return true; };
   virtual bool is_plus_go()    const override { return false;  };
   virtual bool is_any_stop()   const override { return true; };
   virtual bool is_clear()      const override { return false;  };
   virtual bool is_bad()        const override { return true; };
   virtual bool is_clear_for_dir( int dir ) const override { return false; };
   virtual char toChar() const override { return 'X'; };
};

#endif

