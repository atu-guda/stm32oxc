#include <ff.h>
#include <stdlib.h>

// Use dynamic memory allocation is used
#if FF_USE_LFN == 3


void* ff_memalloc ( UINT msize )
{
  return malloc( (size_t)msize );
}


void ff_memfree ( void* mblock )
{
  free( mblock );
}

#endif




#if FF_FS_REENTRANT
#include <FreeRTOS.h>
#include "semphr.h"
static SemaphoreHandle_t Mutex[FF_VOLUMES + 1];	/* Table of mutex handle */

// Returns 1:Function succeeded or 0:Could not create the mutex
// vol: Mutex ID: Volume mutex (0 to FF_VOLUMES - 1) or system mutex (FF_VOLUMES)
int ff_mutex_create ( int vol )
{
  Mutex[vol] = xSemaphoreCreateMutex();
  return (int)( Mutex[vol] != NULL );
}


void ff_mutex_delete ( int vol )
{
  vSemaphoreDelete(Mutex[vol]);
}


/* Returns 1:Succeeded or 0:Timeout */
int ff_mutex_take ( int vol )
{
  return (int)( xSemaphoreTake(Mutex[vol], FF_FS_TIMEOUT) == pdTRUE );
}


void ff_mutex_give( int vol )
{
  xSemaphoreGive( Mutex[vol] );
}

#endif	/* FF_FS_REENTRANT */

