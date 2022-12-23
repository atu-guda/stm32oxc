#ifndef _OXC_TICKER_H
#define _OXC_TICKER_H

class OxcTicker {
  public:
    explicit OxcTicker( int aw )   : w( aw ),    pw( &w )           { start(); } ;
    OxcTicker( volatile int *apw, int qsz ) : w( *apw ), pw( apw ), q( qsz ) { start(); } ;
    bool isTick();
    void start();
    void setW( int aw ) { w = aw; }
  protected:
    int w, next;
    volatile int *pw;
    int q = 1;
};


#endif

