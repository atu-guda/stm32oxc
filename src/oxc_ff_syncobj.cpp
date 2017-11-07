#include <FreeRTOS.h>
#include <semphr.h>
#include <ff.h>

// functions for FreeRTOS sync for FATFS

int ff_cre_syncobj ( BYTE vol, _SYNC_t *sobj ) // 0 = fail
{
  *sobj = xSemaphoreCreateMutex();
  return ( sobj != nullptr );
}

int ff_del_syncobj( _SYNC_t sobj )
{
  if( !sobj ) {
    return false;
  }
  vQueueDelete( sobj );
  return true;
}


int ff_req_grant( _SYNC_t sobj )
{
  return xSemaphoreTake( sobj, _FS_TIMEOUT );
}


void ff_rel_grant( _SYNC_t sobj )
{
  xSemaphoreGive( sobj );
}

