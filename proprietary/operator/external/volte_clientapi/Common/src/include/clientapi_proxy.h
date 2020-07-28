#ifndef __CLIENTAPI_PROXY_H__
#define __CLIENTAPI_PROXY_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "clientapi_comm.h"

//CLIENTAPI PROXY FILE HANDLES AS A MEDIATOR BETWEEN THE CLIENTAPI_ADAPTER AND CLIENTAPI_DISPATCHER
//AND HELP PASS THE MESSAGES BETWEEN THEM

int handleEventFromDispatcher(CLIENTAPI_EventMsg_t *event);
int handleEventFromAdapter(CLIENTAPI_EventMsg_t *event);

#endif /* __CLIENTAPI_PROXY_H__ */ /** __CLIENTAPI_PROXY_H__  **/

