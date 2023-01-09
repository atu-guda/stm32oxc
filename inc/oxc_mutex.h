#ifndef _OXC_MUTEX_H
#define _OXC_MUTEX_H

#include <oxc_base.h>


void mu_lock( mu_t *m );
int  mu_trylock( mu_t *m ); // 0 - ok, like pthread
int  mu_waitlock( mu_t *m, uint32_t ms );
void mu_unlock( mu_t *m );


#ifdef USE_OXC
#define Mu_t  mu_t
#define Mu_lock(x)       mu_lock(x)
#define Mu_unlock(x)     mu_unlock(x)
#define Mu_trylock(x)    mu_trylock(x)
#define Mu_init          0
#else
#include <pthread.h>
#define Mu_t  pthread_mutex_t
#define Mu_lock(x)       pthread_mutex_lock(x)
#define Mu_unlock(x)     pthread_mutex_unlock(x)
#define Mu_trylock(x)    pthread_mutex_trylock(x)
#define Mu_init          PTHREAD_MUTEX_INITIALIZER
#endif

class MuLock {
  public:
   MuLock( Mu_t &a_mu ) : mu( a_mu ) { Mu_lock( &mu ); };
   MuLock( const MuLock &rhs ) = delete;
   ~MuLock()  { Mu_unlock( &mu ); };
   MuLock& operator=( const MuLock &rhs ) = delete;
  protected:
   Mu_t &mu;
};

class MuTryLock {
  public:
   MuTryLock( Mu_t &a_mu ) : mu( a_mu ), acq( !Mu_trylock( &mu ) ) { };
   MuTryLock( const MuLock &rhs ) = delete;
   ~MuTryLock()  { if( acq ) { Mu_unlock( &mu ); } };
   bool wasAcq() const { return acq; }
   MuTryLock& operator=( const MuTryLock &rhs ) = delete;
  protected:
   Mu_t &mu;
   const bool acq;
};

#endif


// vim: path=.,/usr/share/stm32cube/inc
