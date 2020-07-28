
#ifdef __cplusplus
extern "C" {
#endif


/*** STANDARD INCLUDES *******************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include <sys/un.h>


/*** PROJECT INCLUDES ********************************************************/
#include "ports.h"
#include "mi_types.h"
#include "mi_util.h"

#include "powerd_cmd.h"
#include "powerd_core.h"
#include "power_ipc.h"


/*** MACROS ******************************************************************/
#define _TCP_BIND_ADDR_ "com.mediatek.powerhald"


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PRIVATE TYPES DEFINITIONS ***********************************************/


/*** PRIVATE VARIABLE DECLARATIONS (STATIC) **********************************/


/*** PRIVATE FUNCTION PROTOTYPES *********************************************/
static int _connect_powerd(void)
{
   int vRet;
   vRet = socket_local_client(_TCP_BIND_ADDR_, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);

   if (vRet < 0)
   {
      TWPCDBGP("Connect to %s failed.\n", _TCP_BIND_ADDR_);
   }

   return vRet;
}

static int _waitRsploop(tPS_IPC_RecvJob * pRecvJob, int CliFD, int Timeout)
{
   int vRet;

   pRecvJob->State = PS_IPC_COM_NULL;

   while (pRecvJob->State == PS_IPC_COM_NULL)
   {
      vRet = powerd_ipc_recvcmd(pRecvJob, CliFD);

      if (vRet == -1)
      {
         // socket error, release
         TWPCDBGP("timeout:%d\n", Timeout);

         return -1;
      }
   }

   return 0;
}


/*** PUBLIC FUNCTION DEFINITIONS *********************************************/
int power_cmd_create(void ** ppCmd)
{
   if (!ppCmd)
      return 1;

   return powerd_cmd_create(ppCmd);
}

int power_cmd_destory(void * pCmd)
{
   if (!pCmd)
      return 1;

   return powerd_cmd_destory(pCmd);
}

long power_msg(void * pMsg, void **ppRspMsg)
{
   long vRet = 0;
   int vFD;
   tPS_IPC_SendJob vSendJob;
   tPS_IPC_RecvJob vRecvJob;

   struct tPS_CMD * vpCmd, * vpRspCmd;

   power_cmd_create((void **) &vpCmd);

   vpCmd->CmdID = PS_IPC_COM_TYPE_MSG;
   vpCmd->pMSG = pMsg;

   memset(&vSendJob, 0, sizeof(tPS_IPC_SendJob));
   memset(&vRecvJob, 0, sizeof(tPS_IPC_RecvJob));

   powerd_cmd_marshall(&vSendJob, vpCmd);

   vFD = _connect_powerd();
   if (vFD < 0)
   {
      vRet = -1;
      goto exit;
   }

   powerd_ipc_sendcmd(&vSendJob, vFD);

   // wait rsp
   _waitRsploop(&vRecvJob, vFD, 10);

   // unmarshall to cmd
   vRet = powerd_cmd_unmarshall(&vRecvJob, &vpRspCmd);

   if (vRet)
   {
      vRet = -1;
      goto exit;
   }

//   vRet = vpRspCmd->Handle;

   *ppRspMsg = vpRspCmd->pMSG;

   powerd_cmd_destory(vpRspCmd);

exit:
   if (vFD >= 0)
   {
      close(vFD);
   }

   powerd_cmd_destory(vpCmd);

   powerd_ipc_resetSendJob(&vSendJob);
   powerd_ipc_resetRecvJob(&vRecvJob);

   return vRet;
}

#ifdef __cplusplus
}
#endif

