#include <cstring>

#include <oxc_smallrl_q.h>
// #include <oxc_debug1.h>

#include <task.h>

using namespace std;
using namespace SMLRL;

void task_smallrl_cmd( void * /*prm*/ )
{
  struct SmallRlCmd cmd;  BaseType_t ts;
  while(1) {
    ts = xQueueReceive( smallrl_cmd_queue, &cmd, 2000 );
    if( ts == pdTRUE ) {
      // pr( NL "task_smallrl_cmd:" NL );
      // dump8( cmd.cmdline,  cmd.l+1 );
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
  static SmallRlCmd cmd;
  if( l >= (int)sizeof( cmd.cmdline ) ) {
    l = sizeof( cmd.cmdline ) - 1;
  }

  cmd.l = l;
  cmd.cmdline[l+1] = '\0';
  memmove( cmd.cmdline, s, l+1 );
  // pr( NL "exec_queue: sizeof(SmallRlCmd)= "  ); pr_d( sizeof(SmallRlCmd) ); pr( NL );
  // dump8( cmd.cmdline,  cmd.l+1 );
  BaseType_t taskWoken;
  return xQueueSendFromISR( smallrl_cmd_queue, &cmd, &taskWoken );
}

