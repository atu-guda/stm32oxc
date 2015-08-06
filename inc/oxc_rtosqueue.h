#ifndef _OXC_OXC_RTOSQUEUE_H
#define _OXC_OXC_RTOSQUEUE_H

#include <oxc_base.h>

#include <FreeRTOS.h>
#include <queue.h>

class RtosQueue {
  public:
   RtosQueue( unsigned len, unsigned elmsz );
   RtosQueue( const RtosQueue &r ) = delete;
   ~RtosQueue();
   RtosQueue& operator=( const RtosQueue &rhs ) = delete;
   QueueHandle_t handle() { return h; }
   void reset() { xQueueReset( h ); }
   bool isGood() const { return h != nullptr; };
   bool send( const void *ptr, TickType_t tout = 1 );
   bool sendFromISR( const void *ptr, BaseType_t *woken );
   bool recv( void *ptr, TickType_t tout = 1 );
   bool recvFromISR( void *ptr, BaseType_t *woken );
  private:
   QueueHandle_t h;
};



#endif

