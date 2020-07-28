
#ifdef __cplusplus
extern "C" {
#endif

/*** STANDARD INCLUDES *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>


/*** PROJECT INCLUDES ********************************************************/
#include "mi_types.h"
#include "mi_util.h"
#include "ports.h"
#include "ptimer.h"

#include "powerd_core.h"
#include "power_ipc.h"
#include "powerd_cmd.h"


/*** MACROS ******************************************************************/
#define PS_IPC_COM_EVENTBUF_LEN   1024

#define PS_IPC_COM_CONTENT_MAX_LEN 65535

#define PS_SCN_CFG_TYPE_LEN 2
#define PS_SCN_CFG_LEN_LEN  2


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PRIVATE TYPES DEFINITIONS ***********************************************/
typedef int (*tConfigHandler) (char * pData, int Datalen, tPS_CMD * pCmd, int * pUsedLen);

typedef struct tConfigHanderMap
{
   int Idx;
   tConfigHandler pHandler;
} tConfigHanderMap;

static int _PS_Cfg_Hdler_MEMORY(char * pData, int Datalen, tPS_CMD * pCmd, int * pUsedLen);

typedef int (*tConfigHandler_marshallbuf_len) (tPS_CMD * pCmd, int * pBufferLen);
typedef int (*tConfigHandler_marshall) (tPS_CMD * pCmd, char * pBuffer, int BufferLen, int * pUsedLen);

typedef struct tConfigHanderMarshallMap
{
   int Idx;
   tConfigHandler_marshallbuf_len pBufLenHandler;
   tConfigHandler_marshall pMarshallHandler;
} tConfigHanderMarshallMap;

static int _PS_Cfg_Hdler_MarBufLen_MEMORY(tPS_CMD * pCmd, int * pBufferLen);

static int _PS_Cfg_Hdler_Mar_MEMORY(tPS_CMD * pCmd, char * pBuffer, int BufferLen, int * pUsedLen);


/*** PRIVATE VARIABLE DECLARATIONS (STATIC) **********************************/
static tConfigHanderMap _ConfigHandlerMapper[] =
{
   {PS_SCN_TYPE_MEMORY, _PS_Cfg_Hdler_MEMORY},
};

static const int _CntofConfigHandlerMapper = sizeof(_ConfigHandlerMapper)/sizeof(struct tConfigHanderMap);

static tConfigHanderMarshallMap _ConfigHandlerMarshallMapper[] =
{
   {PS_SCN_TYPE_MEMORY, _PS_Cfg_Hdler_MarBufLen_MEMORY, _PS_Cfg_Hdler_Mar_MEMORY},
};

static const int _CntofConfigHandlerMarshallMapper = sizeof(_ConfigHandlerMarshallMapper)/sizeof(struct tConfigHanderMarshallMap);


/*** PRIVATE FUNCTION PROTOTYPES *********************************************/
static int _PS_Cfg_Hdler_MarBufLen_MEMORY(tPS_CMD * pCmd, int * pBufferLen)
{
   *pBufferLen = 0;

   if (pCmd->pMSG != NULL)
   {
      *pBufferLen = 2 + 2 + sizeof(pCmd->pMSG);
   }

   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
static int _PS_Cfg_Hdler_Mar_MEMORY(tPS_CMD * pCmd, char * pBuffer, int BufferLen, int * pUsedLen)
{
   unsigned short vU16;

   *pUsedLen = 0;

   if (pCmd->pMSG == NULL || BufferLen < 8)
   {
      return 0;
   }

   vU16 = htons(PS_SCN_TYPE_MEMORY);
   memcpy(pBuffer, (char *) (&vU16), 2);

   vU16 = htons(sizeof(pCmd->pMSG));
   memcpy(pBuffer + 2, &vU16, 2);

   memcpy(pBuffer + 4, &(pCmd->pMSG), sizeof(pCmd->pMSG));

   //TWPCDBGP("SCN T L: %x %x %x %x\n", *(pBuffer), *(pBuffer + 1), *(pBuffer + 2), *(pBuffer + 3));
   //TWPCDBGP("SCN V: %x %x %x %x\n", *(pBuffer + 4), *(pBuffer + 5), *(pBuffer + 6), *(pBuffer + 7));

   *pUsedLen = 2 + 2 + sizeof(pCmd->pMSG);

   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
static int _PS_Cfg_Hdler_MEMORY(char * pData, int Datalen, tPS_CMD * pCmd, int * pUsedLen)
{
   unsigned short vLen;

   memcpy(&vLen, pData + PS_SCN_CFG_TYPE_LEN, PS_SCN_CFG_LEN_LEN);

   vLen = ntohs(vLen);

   if (vLen != sizeof(pCmd->pMSG) || Datalen < vLen + PS_SCN_CFG_TYPE_LEN + PS_SCN_CFG_LEN_LEN)
   {
         TWPCDBGP("_PS_Cfg_Hdler_MEMORY\n");
      // to do, error check
   }
   else
   {
      memcpy(&pCmd->pMSG, pData + PS_SCN_CFG_TYPE_LEN + PS_SCN_CFG_LEN_LEN, vLen);
   }

   *pUsedLen = PS_SCN_CFG_TYPE_LEN + PS_SCN_CFG_LEN_LEN + vLen;

   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
static int _power_ipc_type_handler_msg(tPS_IPC_RecvJob * pJob, tPS_CMD ** ppCmd)
{
   tPS_CMD * vpCmd = NULL;
   int vI;
   unsigned long vHandledLen = 0;
   char * vpCfg;
   unsigned short vCfgType;

   int vUsedLen;
   int vRet;

   vRet = powerd_cmd_create((void **) &vpCmd);
   if (vRet)
      return -1;

   vpCmd->CmdID = pJob->CAMCOMType;

   vpCfg = pJob->pContent;

   while (vHandledLen < pJob->ContentLen)
   {
      vUsedLen = 0;

      memcpy(&vCfgType, vpCfg, PS_SCN_CFG_TYPE_LEN);

      vCfgType = ntohs(vCfgType);

      //TWPCDBGP("%d\n", vCfgType);

      for (vI = 0; vI < _CntofConfigHandlerMapper; vI++)
      {
         if (_ConfigHandlerMapper[vI].Idx == vCfgType)
         {
            vRet = _ConfigHandlerMapper[vI].pHandler(vpCfg, pJob->ContentLen - vHandledLen, vpCmd, &vUsedLen);

            if (vRet)
            {
               // to do, error check
               TWPCDBGP("_power_ipc_type_handler_msg\n");
            }
            else
            {
               vHandledLen += vUsedLen;
               vpCfg = vpCfg + vUsedLen;
            }

            break;
         }
      }

      if (vI == _CntofConfigHandlerMapper)
      {
         TWPCDBGP("vCfgType:%d not found\n", vCfgType);
         break;
      }
   }

   *ppCmd = vpCmd;

   return 0;
}


/*** PUBLIC FUNCTION DEFINITIONS *********************************************/
int powerd_ipc_init_pscmd(tPS_CMD * pCmd)
{
   pCmd->pMSG = NULL;

   return 0;
}

int powerd_ipc_recvcmd(tPS_IPC_RecvJob * pJob, int PSDc_fd)
{
   unsigned long vWantRecvLen;
   int vRet = 0;

   if (pJob == NULL)
   {
      return -1;
   }

   if (pJob->State == PS_IPC_COM_READY)
   {
      return 0;
   }

SecondRound:

   if (pJob->RecvLen < PS_IPC_COM_HEADER_LEN)
   {
      vWantRecvLen = PS_IPC_COM_HEADER_LEN - pJob->RecvLen;

      vRet = recv(PSDc_fd, pJob->Header + pJob->RecvLen, vWantRecvLen, 0);
   }
   else if (pJob->ContentLen > 0)
   {
      vWantRecvLen = pJob->ContentLen + PS_IPC_COM_HEADER_LEN - pJob->RecvLen;

      vRet = recv(PSDc_fd, pJob->pContent + (pJob->RecvLen - PS_IPC_COM_HEADER_LEN), vWantRecvLen, 0);
   }
   else
   {
      // should not happen
      TWPCDBGP("powerd_ipc_recvcmd should not happen\n");
      return -1;
   }

   //TWPCDBGP("vWantRecvLen %lu, vRet %d\n", vWantRecvLen, vRet);

   if (vRet <= 0)
   {
      TWPCDBGP("powerd_ipc_recvcmd vRet %d errno %d, %s\n", vRet, errno, strerror(errno));

      #if 0
      if (vRet == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
      {
         return 0;
      }
      #endif

      return -1;
   }

   if (pJob->RecvLen < PS_IPC_COM_HEADER_LEN)
   {
      unsigned long vLenParam;

      pJob->RecvLen += vRet;

      if (pJob->RecvLen < PS_IPC_COM_HEADER_LEN)
      {
         TWPCDBGP("powerd_ipc_recvcmd pJob->RecvLen < PS_IPC_COM_HEADER_LEN\n");
         return -1;
      }

      memcpy(&vLenParam, pJob->Header + 4, 4);

      vLenParam = ntohl(vLenParam);

      if (vLenParam <= PS_IPC_COM_CONTENT_MAX_LEN)
      {
         if (vLenParam > 0)
         {
            pJob->pContent = (char *) malloc(vLenParam);
            pJob->ContentLen = vLenParam;

            goto SecondRound;
         }
      }
      else
      {
         TWPCDBGP("powerd_ipc_recvcmd vLenParam > PS_IPC_COM_CONTENT_MAX_LEN\n");
         return -1;
      }
   }
   else if (pJob->ContentLen > 0)
   {
      pJob->RecvLen += vRet;

      vWantRecvLen = pJob->ContentLen + PS_IPC_COM_HEADER_LEN - pJob->RecvLen;

      if (vWantRecvLen > 0)
      {
         TWPCDBGP("powerd_ipc_recvcmd unexpected vWantRecvLen:%lu\n", vWantRecvLen);
         return -1;
      }
   }

   memcpy(&pJob->CAMCOMType, pJob->Header + 2, 2);

   pJob->CAMCOMType = ntohs(pJob->CAMCOMType);
   pJob->State = PS_IPC_COM_READY;

   return 0;
}

int powerd_ipc_sendcmd(tPS_IPC_SendJob * pJob, int PSDc_fd)
{
    if(send(PSDc_fd, pJob->pData, pJob->DataLen, MSG_NOSIGNAL) < 0)
        return -1;

    return 0;
}

int powerd_cmd_unmarshall(tPS_IPC_RecvJob * pJob, tPS_CMD ** ppCmd)
{
   *ppCmd = NULL;

   // to do, check job's state

   switch (pJob->CAMCOMType)
   {
      case PS_IPC_COM_TYPE_MSG:
         _power_ipc_type_handler_msg(pJob, ppCmd);
         break;

      default:
         // to do, may be registerd by others, for now just error
         TWPCDBGP("Unhandled Command Type %d at cmd_unmarshall\n", pJob->CAMCOMType);
         return -1;
         break;
   }

   return 0;
}

int powerd_cmd_marshall(tPS_IPC_SendJob * pJob, tPS_CMD * pCmd)
{
   int vI;
   int vRet;
   int vBufLen = 8;
   char * vpBuf = NULL;

   char * vpCfgBuf;
   int vCfgBufLen;
   int vCfgUsedLen;

   unsigned short vU16;
   unsigned long vU32;

   // 1. Predict buffer len
   for (vI = 0; vI < _CntofConfigHandlerMarshallMapper; vI++)
   {
      vRet = _ConfigHandlerMarshallMapper[vI].pBufLenHandler(pCmd, &vCfgUsedLen);

      vBufLen += vCfgUsedLen;
   }

   if(vBufLen > 0)
      vpBuf = (char *) malloc(vBufLen);

   if (vpBuf == NULL)
      return -1;

   // keep header
   vpCfgBuf = vpBuf + 8;
   vCfgBufLen = vBufLen - 8;

   // marshall to buffer
   // 1. Predict buffer len
   for (vI = 0; vI < _CntofConfigHandlerMarshallMapper; vI++)
   {
      vRet = _ConfigHandlerMarshallMapper[vI].pMarshallHandler(pCmd, vpCfgBuf, vCfgBufLen, &vCfgUsedLen);

      vpCfgBuf += vCfgUsedLen;
      vCfgBufLen -= vCfgUsedLen;
   }

   // write header
   memset(vpBuf, 0, 2);

   vU16 = htons(pCmd->CmdID);
   memcpy(vpBuf + 2, &vU16, 2);

   vU32 = htonl(vBufLen - 8);
   memcpy(vpBuf + 4, &vU32, 4);

   // fill SendJob
   pJob->pData = vpBuf;
   pJob->DataLen = vBufLen;

   return 0;
}

int powerd_ipc_resetRecvJob(tPS_IPC_RecvJob * pJob)
{
   if (pJob->pContent)
      free(pJob->pContent);

   pJob->pContent = NULL;
   pJob->ContentLen = 0;

   return 0;
}

int powerd_ipc_resetSendJob(tPS_IPC_SendJob * pJob)
{
   if (pJob->pData)
      free(pJob->pData);

   pJob->pData = NULL;
   pJob->DataLen = 0;

   return 0;
}


#ifdef __cplusplus
}
#endif

