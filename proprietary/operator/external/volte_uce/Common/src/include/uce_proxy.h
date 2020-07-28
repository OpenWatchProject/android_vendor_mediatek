#ifndef __UCE_PROXY_H__
#define __UCE_PROXY_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "uce_comm.h"

//RCS PROXY FILE HANDLES AS A MEDIATOR BETWEEN THE RCS_ADAPTER AND RCS_DISPATCHER
//AND HELP PASS THE MESSAGES BETWEEN THEM

int handleEventFromDispatcher(UCE_EventMsg_t *event);
int handleEventFromAdapter(UCE_EventMsg_t *event);

#endif /* __UCE_PROXY_H__ */ /** __UCE_PROXY_H__  **/

