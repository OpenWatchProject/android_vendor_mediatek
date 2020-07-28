#ifndef __RCS_PROXY_H__
#define __RCS_PROXY_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "rcs_comm.h"

//RCS PROXY FILE HANDLES AS A MEDIATOR BETWEEN THE RCS_ADAPTER AND RCS_DISPATCHER
//AND HELP PASS THE MESSAGES BETWEEN THEM

/* handles events from the dispatcher */
extern int handleEventFromDispatcher(RCS_EventMsg_t *event);

/* handles events from the rcs adapter */
extern int handleEventFromAdapter(RCS_EventMsg_t *event);

#endif /* __RCS_PROXY_H__ */ /** __RCS_PROXY_H__  **/

