#ifndef _OXC_SMALLRL_Q_H
#define _OXC_SMALLRL_Q_H

#include <oxc_smallrl.h>

#include <FreeRTOS.h>
#include <queue.h>

struct SmallRlCmd {
  int l;
  char cmdline[SMLRL_BUFSZ];
};
extern QueueHandle_t smallrl_cmd_queue;

#define SMALLRL_INIT_QUEUE smallrl_cmd_queue = xQueueCreate( 1, sizeof(SmallRlCmd) );

extern "C" {
void task_smallrl_cmd( void *prm );
};

namespace SMLRL {
  int exec_queue( const char *s, int l );
};


#endif

