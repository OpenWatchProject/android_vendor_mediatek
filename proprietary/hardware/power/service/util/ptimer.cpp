
#ifdef __cplusplus
extern "C" {
#endif

/*** STANDARD INCLUDES *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*** PROJECT INCLUDES ********************************************************/
#include "mi_types.h"
#include "mi_util.h"
#include "ports.h"
#include "ptimer.h"


/*** MACROS ******************************************************************/


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PRIVATE TYPES DEFINITIONS ***********************************************/
typedef struct _tPTIMER_MNG_
{
   tMI_DLIST List;
} _tPTIMER_MNG_;

typedef struct _tPTIMER_TIMER_
{
   tMI_DLNODE Node;

   _tPTIMER_MNG_ *pMng;

   void * pData;

   unsigned long expire_sec;
   unsigned long expire_usec;
} _tPTIMER_TIMER_;


/*** PRIVATE VARIABLE DECLARATIONS (STATIC) **********************************/


/*** PRIVATE FUNCTION PROTOTYPES *********************************************/
const void * _keyofPTIMER_(const tMI_DLNODE * pNode)
{
   return (void *) MI_NODEENTRY(pNode, _tPTIMER_TIMER_, Node);
}

S32 _comparePTIMER_(const void * ptarget, const void * pexist)
{
   _tPTIMER_TIMER_ * vpTarget = (_tPTIMER_TIMER_ *) ptarget;
   _tPTIMER_TIMER_ * vpExist = (_tPTIMER_TIMER_ *) pexist;

   if (vpTarget->expire_sec > vpExist->expire_sec)
   {
      return -1;
   }
   else if (vpTarget->expire_sec < vpExist->expire_sec)
   {
      return 1;
   }

   if (vpTarget->expire_usec > vpExist->expire_usec)
   {
      return -1;
   }

   return 0;
}


/*** PUBLIC FUNCTION DEFINITIONS *********************************************/
int ptimer_mng_create(void ** ppmng)
{
   _tPTIMER_MNG_ * vpMNG = NULL;

   if (ppmng == NULL)
   {
      return -1;
   }

   *ppmng = NULL;

   vpMNG = (_tPTIMER_MNG_ *) pmalloc(sizeof(_tPTIMER_MNG_)); pmemset(vpMNG, 0, sizeof(_tPTIMER_MNG_));

   if (vpMNG == NULL)
   {
      return -1;
   }

   MI_DlInit(&vpMNG->List);

   *ppmng = vpMNG;

   return 0;
}

int ptimer_mng_delete(void * pmng)
{
   _tPTIMER_MNG_ * vpMNG = (_tPTIMER_MNG_ *) pmng;

   if (pmng == NULL)
   {
      return -1;
   }

   if (MI_DlCount(&vpMNG->List))
   {
      return -1;
   }

   pfree(pmng);

   return 0;
}

int ptimer_mng_getnextduration(void * pmng, unsigned long * pmseconds)
{
   _tPTIMER_MNG_ * vpMNG = (_tPTIMER_MNG_ *) pmng;
   tMI_DLNODE * vpNode = NULL;
   _tPTIMER_TIMER_ * vpTimer = NULL;

   unsigned long vNowSec;
   unsigned long vNowNSec;

   if (pmng == NULL || pmseconds == NULL)
   {
      return -1;
   }

   *pmseconds = 0;

   if (MI_DlCount(&vpMNG->List) == 0)
   {
      return 1;
   }

   vpNode = MI_DlFirst(&vpMNG->List);
   vpTimer = MI_NODEENTRY(vpNode, _tPTIMER_TIMER_, Node);

   pwalltime(&vNowSec, &vNowNSec);

   if (vNowSec > vpTimer->expire_sec)
   {
      return 0;
   }
   else if (vNowSec == vpTimer->expire_sec && vNowNSec / 1000 > vpTimer->expire_usec)
   {
      return 0;
   }

   *pmseconds += (vpTimer->expire_sec - vNowSec) * 1000;

   *pmseconds += vpTimer->expire_usec / 1000 - vNowNSec /1000000;

   return 0;
}

int ptimer_mng_getexpired(void * pmng, void ** pptimer, void ** ppdata, unsigned long * pWallSec, unsigned long * pWallNSec)
{
   _tPTIMER_MNG_ * vpMNG = (_tPTIMER_MNG_ *) pmng;
   tMI_DLNODE * vpNode = NULL;
   _tPTIMER_TIMER_ * vpTimer = NULL;

   unsigned long vWallSec;
   unsigned long vWallNSec;

   if (pmng == NULL || pptimer == NULL || ppdata == NULL)
   {
      return -1;
   }

   *pptimer = NULL;
   *ppdata = NULL;

   if (MI_DlCount(&vpMNG->List) == 0)
   {
      return 0;
   }

   //TCPVSDBGP("%x\n", &vpMNG->List);

   vpNode = MI_DlFirst(&vpMNG->List);
   vpTimer = MI_NODEENTRY(vpNode, _tPTIMER_TIMER_, Node);

   pwalltime(&vWallSec, &vWallNSec);

   if (pWallSec)
   {
      *pWallSec = vWallSec;
   }

   if (pWallNSec)
   {
      *pWallNSec = vWallNSec;
   }

   if (vWallSec > vpTimer->expire_sec)
   {
      MI_DlDelete(&vpMNG->List, vpNode);
      *pptimer = vpTimer;
      *ppdata = vpTimer->pData;

      vpTimer->pMng = NULL;

      return 1;
   }
   else if (vWallSec == vpTimer->expire_sec && (vWallNSec / 1000 > vpTimer->expire_usec || (vpTimer->expire_usec - vWallNSec / 1000) < 1000))
   {
      MI_DlDelete(&vpMNG->List, vpNode);
      *pptimer = vpTimer;
      *ppdata = vpTimer->pData;

      vpTimer->pMng = NULL;

      return 1;
   }

   return 0;
}

int ptimer_create(void ** pptimer)
{
   _tPTIMER_TIMER_ * vpTimer = NULL;

   if (pptimer == NULL)
   {
      return -1;
   }

   vpTimer = (_tPTIMER_TIMER_ *) pmalloc(sizeof(_tPTIMER_TIMER_)); pmemset(vpTimer, 0, sizeof(_tPTIMER_TIMER_));

   if (vpTimer == NULL)
   {
      return -1;
   }

   *pptimer = vpTimer;

   return 0;
}

int ptimer_start(void * pmng, void * ptimer, unsigned long mseconds, void * pdata)
{
   _tPTIMER_MNG_ * vpMNG = (_tPTIMER_MNG_ *) pmng;
   _tPTIMER_TIMER_ * vpTimer = (_tPTIMER_TIMER_ *) ptimer;
   unsigned long vNowSec;
   unsigned long vNowNSec;

   if (ptimer == NULL || pmng == NULL)
   {
      return 0;
   }

   if (vpTimer->pMng)
   {
      MI_DlDelete(&vpTimer->pMng->List, &vpTimer->Node);
      vpTimer->pMng = NULL;
   }

   pwalltime(&vNowSec, &vNowNSec);

   vpTimer->expire_sec = vNowSec + mseconds/1000;
   vpTimer->expire_usec = vNowNSec/1000 + (mseconds % 1000) * 1000;
   if (vpTimer->expire_usec/1000000 >= 1)
   {
      vpTimer->expire_sec++;
      vpTimer->expire_usec = vpTimer->expire_usec % 1000000;
   }

   vpTimer->pData = pdata;
   vpTimer->pMng = vpMNG;

   // insert and sort
   MI_DlInsert(&vpMNG->List, &vpTimer->Node, _keyofPTIMER_, _comparePTIMER_);

   return 0;
}

int ptimer_stop(void * ptimer)
{
   _tPTIMER_TIMER_ * vpTimer = (_tPTIMER_TIMER_ *) ptimer;

   if (ptimer == NULL)
   {
      return 0;
   }

   if (vpTimer->pMng)
   {
      MI_DlDelete(&vpTimer->pMng->List, &vpTimer->Node);
      vpTimer->pMng = NULL;
   }

   return 0;
}

int ptimer_delete(void * ptimer)
{
   _tPTIMER_TIMER_ * vpTimer = (_tPTIMER_TIMER_ *) ptimer;

   if (ptimer == NULL)
   {
      return 0;
   }

   if (vpTimer->pMng)
   {
      MI_DlDelete(&vpTimer->pMng->List, &vpTimer->Node);
      vpTimer->pMng = NULL;
   }

   pfree(vpTimer);

   return 0;
}

int ptimer_util_walltimepass_ms(unsigned long OldWallSec, unsigned long OldWallNSec, unsigned long * pmseconds)
{
   unsigned long vNowSec;
   unsigned long vNowNSec;

   if (pmseconds == NULL)
   {
      return -1;
   }

   *pmseconds = 0;

   pwalltime(&vNowSec, &vNowNSec);

   if (vNowSec < OldWallSec)
   {
      return 0;
   }
   else if (vNowSec == OldWallSec && vNowNSec <= OldWallNSec)
   {
      return 0;
   }

   *pmseconds += (vNowSec - OldWallSec) * 1000;

   *pmseconds += vNowNSec / 1000000 - OldWallNSec / 1000000;

   return 0;
}

#ifdef __cplusplus
}
#endif


