#include <oxc_rtosqueue.h>

RtosQueue::RtosQueue( unsigned len, unsigned elmsz )
  : h( xQueueCreate( len, elmsz ) )
{
}


RtosQueue::~RtosQueue()
{
  vQueueDelete( h ); h = nullptr;
}


bool RtosQueue::send( const void *ptr, TickType_t tout  )
{
  if( !h || !ptr ) {
    return false;
  }
  return xQueueSend( h, ptr, tout );
}


bool RtosQueue::sendFromISR( const void *ptr, BaseType_t *woken )
{
  if( !h || !ptr ) {
    return false;
  }
  return xQueueSendFromISR( h, ptr, woken );
}


bool RtosQueue::recv( void *ptr, TickType_t tout  )
{
  if( !h || !ptr ) {
    return false;
  }
  return xQueueReceive( h, ptr, tout );

}


bool RtosQueue::recvFromISR( void *ptr, BaseType_t *woken )
{
  if( !h || !ptr ) {
    return false;
  }
  return xQueueReceiveFromISR( h, ptr, woken );
}


