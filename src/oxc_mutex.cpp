#include <oxc_mutex.h>


void mu_lock( mu_t *m )
{
  // oxc_disable_interrupts();
  while( mu_trylock( m ) != 0 ) { /* NOP */ ; };
  // oxc_enable_interrupts();
}

int  mu_trylock( mu_t *m )
{
  uint32_t sta = 1;

  if( oxc_ldrex( m ) == 0 ) { // unlocked
    sta = oxc_strex( 1, m ); // try to lock
  }
  oxc_dmb();

  return sta != 0; // return 0 if locked!
}

int  mu_waitlock( mu_t *m, uint32_t ms ) // returns 0 - lock is acquired
{
  for( uint32_t i=0; i<ms; ++i ) {
    if( mu_trylock( m ) == 0 ) {
      return 0;
    }
    delay_ms( 1 );
  }
  return 1;
}


void mu_unlock( mu_t *m )
{
  oxc_dmb();
  *m = 0;
}

