#include <cstring>

#include <oxc_smallrl_q.h>

#include <task.h>

using namespace std;
using namespace SMLRL;

void task_smallrl_cmd( void * /*prm*/ )
{
  struct SmallRlCmd cmd;  BaseType_t ts;
  while(1) {
    ts = xQueueReceive( smallrl_cmd_queue, &cmd, 2000 );
    if( ts == pdTRUE ) {
      exec_direct( cmd.cmdline, cmd.l );
      delay_ms( 10 );
      if( global_smallrl ) {
        global_smallrl->redraw();
      }
    }
  }
  vTaskDelete(0);
}

int SMLRL::exec_queue( const char *s, int l )
{
  SmallRlCmd cmd;
  cmd.l = l;
  memcpy( cmd.cmdline, s, l+1 );
  return xQueueSend( smallrl_cmd_queue, &cmd, 10000  );
}

