#include <FreeRTOS.h>
#include <semphr.h>
#include <ff.h>

// functions for FreeRTOS sync for FATFS

int ff_cre_syncobj ( BYTE vol, FF_SYNC_t *sobj ) // 0 = fail
{
  *sobj = xSemaphoreCreateMutex();
  return ( sobj != nullptr );
}

int ff_del_syncobj( FF_SYNC_t sobj )
{
  if( !sobj ) {
    return false;
  }
  vQueueDelete( sobj );
  return true;
}


int ff_req_grant( FF_SYNC_t sobj )
{
  return xSemaphoreTake( sobj, FF_FS_TIMEOUT );
}


void ff_rel_grant( FF_SYNC_t sobj )
{
  xSemaphoreGive( sobj );
}

