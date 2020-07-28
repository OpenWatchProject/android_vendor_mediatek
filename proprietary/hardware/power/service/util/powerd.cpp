
//#ifdef __cplusplus
//extern "C" {
//#endif

/*** STANDARD INCLUDES *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#include <arpa/inet.h> // ntohs

#include <sys/epoll.h>
#include <netdb.h> // getaddrinfo
#include <sys/inotify.h>

#include <cutils/sockets.h>
#include <cutils/trace.h>

//#define ATRACE_TAG ATRACE_TAG_ALWAYS

/*** PROJECT INCLUDES ********************************************************/
#define PERFD

#include "ports.h"
#include "mi_types.h"
#include "mi_util.h"
#include "ptimer.h"

#include "powerd_int.h"
#include "powerd_core.h"
#include "power_ipc.h"
#include "powerd_cmd.h"


/*** MACROS ******************************************************************/
#define _TCP_PRECONN_TIMER_   1000
#define _TCP_PRECONN_SHORT_TIMER_   500
#define _TCP_CONNVALID_MSECS_   500

#define _TCP_BIND_ADDR_ "com.mediatek.powerhald"

#define _TCP_SKBUF_DATALEN 1500

#define _TCP_SERVER_STATE_NULL          0x00
#define _TCP_SERVER_STATE_RUNNING       0x01

#define _TCP_CLIENT_STATE_NULL          0x00
#define _TCP_CLIENT_STATE_CONNECTING    0x03
#define _TCP_CLIENT_STATE_RUNNING       0x04
#define _TCP_CLIENT_STATE_LINGER        0x05


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/
#ifndef SIM_MAIN
extern int drvb_f0(void);
#endif


/*** PRIVATE TYPES DEFINITIONS ***********************************************/
typedef struct _tTCPVS_Timer
{
   tMI_DLNODE Node;

   void * p_ptimer;

   unsigned int id;
} _tTCPVS_Timer_;

typedef struct _tGlobeContext_
{
   tMI_DLIST TCPCliCtxtList;
   tMI_DLIST TCPHostList;

   int TCPServFD;
   int TCPServState;

   void * pTimerMng;

   unsigned long newlocalacceptSec;
   unsigned long newlocalacceptNSec;

   int inotifyfd;
   int inotifywd;

   int GotUID;
   char UID[_TCP_SKBUF_DATALEN];

   int PreConnTime;
   int isPreConnState;

   _tTCPVS_Timer_ * pPreConnTimer;
} _tGlobeContext_;

typedef struct _tTCPCliContext_
{
   tMI_DLNODE Node;
   tMI_DLNODE HostNode;

   int TCPCliFD;

   int TCPState;

   tPS_IPC_RecvJob RecvJob;
   tPS_IPC_SendJob SendJob;
} _tTCPCliContext_;

typedef struct _tTCPSKBuf_
{
   tMI_DLNODE Node;

   int Length;
   char Data[_TCP_SKBUF_DATALEN];

   int SentLen;
} _tTCPSKBuf_;


/*** PRIVATE VARIABLE DECLARATIONS (STATIC) **********************************/


/*** PRIVATE FUNCTION PROTOTYPES *********************************************/
#ifdef _CHECK_LEAK_
#include <mcheck.h>
static int _Lunch_check_leak(void)
{
   char vTTArr[512];
   struct timeval vvvvtv;

   gettimeofday(&vvvvtv, NULL);
   sprintf(vTTArr, "./memleak_%010lu_%06lu.log", vvvvtv.tv_sec, vvvvtv.tv_usec);

   setenv("MALLOC_TRACE", vTTArr, 1);
   mtrace();

   return 0;
}
#endif

static void _WCPROXY_fillfd(_tGlobeContext_ * pCtxt, fd_set * pRSet, fd_set * pWSet, int * pMaxfd)
{
   tMI_DLNODE * vpNode = NULL;
   _tTCPCliContext_ * vpCliCtxt = NULL;

   // for compile
   if (pWSet)
   {
   }

   if (pCtxt == NULL || pMaxfd == NULL)
   {
      TWPCDBGP("_WCPROXY_fillfd should not happen\n");
      return;
   }

   FD_SET(pCtxt->TCPServFD, pRSet);

   *pMaxfd = pCtxt->TCPServFD;

   vpNode = MI_DlFirst(&pCtxt->TCPCliCtxtList);

   while (vpNode)
   {
      //TWPCDBGP("\n");

      vpCliCtxt = MI_NODEENTRY(vpNode, _tTCPCliContext_, Node);

      if (vpCliCtxt->TCPState != _TCP_CLIENT_STATE_LINGER)
      {
         FD_SET(vpCliCtxt->TCPCliFD, pRSet);

         if (*pMaxfd < vpCliCtxt->TCPCliFD)
         {
            *pMaxfd = vpCliCtxt->TCPCliFD;
         }
      }

      vpNode = MI_DlNext(vpNode);
   }

   *pMaxfd += 1;

   return;
}

static int _acceptNewLocalClient(int Servfd, _tTCPCliContext_ ** ppNewCli)
{
   _tTCPCliContext_ * vpNewCli = NULL;
   int vNewfd;

   if (ppNewCli == NULL)
   {
      return -1;
   }

   *ppNewCli = NULL;

   vpNewCli = (_tTCPCliContext_ *) malloc(sizeof(_tTCPCliContext_));

   if (vpNewCli == NULL)
   {
      return -1;
   }

   vpNewCli->TCPCliFD = -1;

   vNewfd = accept(Servfd, NULL, NULL);
   if (vNewfd < 0)
   {
      free(vpNewCli);
      TWPCDBGP("accept from %d error %d.\n", Servfd, errno);
      return -1;
   }

   if(fcntl(vNewfd, F_SETFL, O_NONBLOCK) == -1) {
      free(vpNewCli);
      close(vNewfd);
      TWPCDBGP("fcntl error %d.\n", Servfd, errno);
      return -1;
   }

   vpNewCli->TCPCliFD = vNewfd;
   vpNewCli->TCPState = _TCP_CLIENT_STATE_RUNNING;

   memset(&vpNewCli->RecvJob, 0, sizeof(tPS_IPC_RecvJob));
   memset(&vpNewCli->SendJob, 0, sizeof(tPS_IPC_SendJob));
   vpNewCli->RecvJob.State = PS_IPC_COM_NULL;

   *ppNewCli = vpNewCli;

   return 0;
}

static void _Powerd_handleIPC(_tGlobeContext_ * pCtxt, _tTCPCliContext_ * pCliCtxt, int isRead, int isWrite)
{
   int vRet;
   int vHandle;
   struct tPS_CMD * vpCmd;

   // for compile
   if (pCtxt)
   {
   }

   if (isRead && pCliCtxt->TCPState == _TCP_CLIENT_STATE_RUNNING)
   {
      vRet = powerd_ipc_recvcmd(&pCliCtxt->RecvJob, pCliCtxt->TCPCliFD);

      if (vRet == -1)
      {
         // socket error, release
         TWPCDBGP("_Powerd_handleIPC socket error\n");
         pCliCtxt->TCPState = _TCP_CLIENT_STATE_LINGER;
         return;
      }
      else if (pCliCtxt->RecvJob.State == PS_IPC_COM_NULL)
      {
         // wait ready
         TWPCDBGP("_Powerd_handleIPC wait ready\n");
         return;
      }

      // unmarshall to cmd
      vRet = powerd_cmd_unmarshall(&pCliCtxt->RecvJob, &vpCmd);

      if (vRet == 0 && vpCmd->CmdID == PS_IPC_COM_TYPE_MSG)
      {
         void * vpRspMsg = NULL;

         // call callback to cmd handler with tPowerData
         vHandle = powerd_req(vpCmd->pMSG, &vpRspMsg);

//         powerd_cmd_destory(vpCmd);
         vpCmd->pMSG = vpRspMsg;

         // send vRet to client
         vpCmd->CmdID = PS_IPC_COM_TYPE_MSG;

         powerd_cmd_marshall(&pCliCtxt->SendJob, vpCmd);

         powerd_cmd_destory(vpCmd);

         powerd_ipc_sendcmd(&pCliCtxt->SendJob, pCliCtxt->TCPCliFD);
      }

      // set linger, wait to close
      pCliCtxt->TCPState = _TCP_CLIENT_STATE_LINGER;
   }
   else
   {
      TWPCDBGP("unhandle Local Cli State 0x%x, isRead %d, isWrite %d\n", pCliCtxt->TCPState, isRead, isWrite);
   }
}

static void _Powerd_scanfd(_tGlobeContext_ * pCtxt, fd_set * pRSet, fd_set * pWSet)
{
   tMI_DLNODE * vpNode = NULL;
   tMI_DLNODE * vpNextNode = NULL;
   _tTCPCliContext_ * vpCliCtxt = NULL;
   int visRead;
   int visWrite;

   vpNode = MI_DlFirst(&pCtxt->TCPCliCtxtList);

   while (vpNode)
   {
      //TWPCDBGP("\n");

      vpCliCtxt = MI_NODEENTRY(vpNode, _tTCPCliContext_, Node);

      if (vpCliCtxt->TCPState == _TCP_CLIENT_STATE_LINGER)
      {
         vpNode = MI_DlNext(vpNode);
         continue;
      }

      visRead = FD_ISSET(vpCliCtxt->TCPCliFD, pRSet);
      visWrite = FD_ISSET(vpCliCtxt->TCPCliFD, pWSet);

      if (visRead || visWrite)
      {
         _Powerd_handleIPC(pCtxt, vpCliCtxt, visRead, visWrite);
      }

      vpNode = MI_DlNext(vpNode);
   }

   if (FD_ISSET(pCtxt->TCPServFD, pRSet))
   {
      _tTCPCliContext_ * vpNewCliCtxt = NULL;

      // accept new local client connect
      if (_acceptNewLocalClient(pCtxt->TCPServFD, &vpNewCliCtxt) == 0)
      {
         MI_DlPushTail(&pCtxt->TCPCliCtxtList, &vpNewCliCtxt->Node);
      }
   }

   // remove LINGER TCPClient
   vpNode = MI_DlFirst(&pCtxt->TCPCliCtxtList);

   while (vpNode)
   {
      //TWPCDBGP("\n");

      vpNextNode = MI_DlNext(vpNode);
      vpCliCtxt = MI_NODEENTRY(vpNode, _tTCPCliContext_, Node);

      if (vpCliCtxt->TCPState == _TCP_CLIENT_STATE_LINGER)
      {
         MI_DlDelete(&pCtxt->TCPCliCtxtList, vpNode);

         // finish the connection
         close(vpCliCtxt->TCPCliFD);

         powerd_ipc_resetRecvJob(&vpCliCtxt->RecvJob);
         powerd_ipc_resetSendJob(&vpCliCtxt->SendJob);

         free(vpCliCtxt);
      }

      vpNode = vpNextNode;
   }

   return;
}

static void _Powerd_handleTimer(_tGlobeContext_ * pCtxt, void * pTimer, void * pData)
{
   // for compile
   if (pCtxt)
   {
   }

   // for compile
   if (pTimer || pData)
   {
      powerd_core_timer_handle(pTimer, pData);
   }
}

static int _selectLoop(_tGlobeContext_ * pCtxt)
{
   fd_set vRSet, vWSet;
   int vMaxfd;
   int vRet;

   struct timeval vtv;
   struct timeval ovtv;

   unsigned long vmsec;
   unsigned long vWallSec;
   unsigned long vWallNSec;
   void * pTimer = NULL;
   void * pData = NULL;

//   _tTCPVS_Timer_ * vpTimer = NULL;

#ifndef SIM_MAIN
//   int r_anti_clone = drvb_f0();
#endif

   vRet = ptimer_mng_create(&pCtxt->pTimerMng);
   powerd_core_init(pCtxt->pTimerMng);

   while (1)
   {
      vMaxfd = -1;

      FD_ZERO(&vRSet);
      FD_ZERO(&vWSet);

      _WCPROXY_fillfd(pCtxt, &vRSet, &vWSet, &vMaxfd);

      vtv.tv_sec = 1;
      vtv.tv_usec = 0;

      vRet = ptimer_mng_getnextduration(pCtxt->pTimerMng, &vmsec);

      if (vRet != 0)
      {
         vtv.tv_sec = 5;
         vtv.tv_usec = 0;
      }
      else
      {
         vtv.tv_usec = (vmsec % 1000) * 1000;
         vtv.tv_sec = vmsec / 1000;
      }

      ovtv = vtv;

      vRet = select(vMaxfd, &vRSet, &vWSet, NULL, &vtv);

      //TWPCDBGP("vMaxfd %d, ovtv sec %lu usec %lu, vtv sec %lu usec %lu, vRet %d", vMaxfd, ovtv.tv_sec, ovtv.tv_usec, vtv.tv_sec, vtv.tv_usec, vRet);

      if (vRet == 0)
      {
         vRet = ptimer_mng_getexpired(pCtxt->pTimerMng, &pTimer, &pData, &vWallSec, &vWallNSec);

         if (vRet == 1)
         {
            _Powerd_handleTimer(pCtxt, pTimer, pData);
         }

         continue;
      }
      else if (vRet < 0)
      {
         TWPCDBGP("_selectLoop vRet %d errno %d, %s\n", vRet, errno, strerror(errno));

         if (errno == EINTR)
         {
            continue;
         }

         break;
      }

      vRet = ptimer_mng_getexpired(pCtxt->pTimerMng, &pTimer, &pData, &vWallSec, &vWallNSec);

      if (vRet == 1)
      {
         _Powerd_handleTimer(pCtxt, pTimer, pData);
      }

      _Powerd_scanfd(pCtxt, &vRSet, &vWSet);
   }

   return 0;
}

static int _slave_main(int argc, char **argv, void (*listen_cb)(void))
{
   int vTCPServFD;
   _tGlobeContext_ vGlobeCtxt;
   int vRet;

   // for compile
   if (argc && argv)
   {
   }

   TWPCDBGP("powerd slave start");

#ifdef _CHECK_LEAK_
   _Lunch_check_leak();
#endif

   //sleep(1);

   MI_DlInit(&vGlobeCtxt.TCPCliCtxtList);

   vTCPServFD = socket_local_server(_TCP_BIND_ADDR_, ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);

   if (vTCPServFD < 0)
   {
      TWPCDBGP("Create unix server socket failed. errno %d, %s\n", errno, strerror(errno));
      return 0;
   }

   if(listen(vTCPServFD, 1024) < 0)
      goto exit;

   /* call listen_cb after listen */
   listen_cb();

   vGlobeCtxt.TCPServFD = vTCPServFD;
   vGlobeCtxt.TCPServState = _TCP_SERVER_STATE_RUNNING;

   vRet = _selectLoop(&vGlobeCtxt);

exit:
   close(vTCPServFD);

   TWPCDBGP("powerd slave end");

   return 0;
}

/*** PUBLIC FUNCTION DEFINITIONS *********************************************/
#if 0
int main(int argc, char **argv)
#else
int powerd_main(int argc, char **argv, void (*listen_cb)(void))
#endif
{
   int vRet;

   TWPCDBGP("powerd start");

   // set nice in service.cpp and PowerManager.cpp
   vRet = nice(-1);

#ifdef _CHECK_LEAK_
   _Lunch_check_leak();
#endif

   _slave_main(argc, argv, listen_cb);

   TWPCDBGP("powerd end");

   return 0;
}


//#ifdef __cplusplus
//}
//#endif

