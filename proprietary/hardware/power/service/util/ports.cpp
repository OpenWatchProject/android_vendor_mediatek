

#ifdef __cplusplus
extern "C" {
#endif

/*** STANDARD INCLUDES *******************************************************/
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if (TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
#include <mach/clock.h>
#include <mach/mach.h>
#endif


/*** PROJECT INCLUDES ********************************************************/
#include "ports.h"


/*** MACROS ******************************************************************/


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PRIVATE TYPES DEFINITIONS ***********************************************/


/*** PRIVATE VARIABLE DECLARATIONS (STATIC) **********************************/


/*** PRIVATE FUNCTION PROTOTYPES *********************************************/


/*** PUBLIC FUNCTION DEFINITIONS *********************************************/
#ifndef _CHECK_LEAK_
void * pmalloc(unsigned int size)
{
   char * vpNew = NULL;

   if(size > 0)
      vpNew = (char *) malloc(size);

   if (vpNew)
   {
      memset((void *) vpNew, 0, size);
   }

   return vpNew;
}

void pfree(void * pPtr)
{
   if (pPtr == NULL)
   {
      return;
   }

   free(pPtr);
}
#endif

int pmutex_create(void ** ppmutex)
{
   pthread_mutex_t * vpmutex = NULL;

   if (ppmutex == NULL)
   {
      return -1;
   }

   *ppmutex = NULL;

   vpmutex = (pthread_mutex_t *) pmalloc(sizeof(pthread_mutex_t));

   if (vpmutex == NULL)
   {
      return -1;
   }

   *ppmutex = (void *) vpmutex;
   return 0;
}

int pmutex_init(void * pmutex)
{
   pthread_mutex_t * vpmutex = (pthread_mutex_t *) pmutex;
   int vRet = 0;

   if (vpmutex == NULL)
   {
      return -1;
   }

   vRet = pthread_mutex_init(vpmutex, NULL);

   if (vRet)
   {
      return -1;
   }

   return 0;
}

int pmutex_destroy(void * pmutex)
{
   pthread_mutex_t * vpmutex = (pthread_mutex_t *) pmutex;

   if (vpmutex == NULL)
   {
      return -1;
   }

   pthread_mutex_destroy(vpmutex);

   pfree(vpmutex);

   return 0;
}

int pmutex_lock(void * pmutex)
{
   pthread_mutex_t * vpmutex = (pthread_mutex_t *) pmutex;
   int vRet = 0;

   if (vpmutex == NULL)
   {
      return -1;
   }

   vRet = pthread_mutex_lock(vpmutex);

   if (vRet)
   {
      return -1;
   }

   return 0;
}

int pmutex_trylock(void * pmutex)
{
   pthread_mutex_t * vpmutex = (pthread_mutex_t *) pmutex;
   int vRet = 0;

   if (vpmutex == NULL)
   {
      return -1;
   }

   vRet = pthread_mutex_trylock(vpmutex);

   if (vRet == EBUSY)
   {
      return 1;
   }
   else
   {
      return -1;
   }

   return 0;
}

int pmutex_unlock(void * pmutex)
{
   pthread_mutex_t * vpmutex = (pthread_mutex_t *) pmutex;
   int vRet = 0;

   if (vpmutex == NULL)
   {
      return -1;
   }

   vRet = pthread_mutex_unlock(vpmutex);

   if (vRet)
   {
      return -1;
   }

   return 0;
}

int pwalltime(unsigned long * pseconds, unsigned long * pnanoseconds)
{
   struct timespec t;

   if (pseconds == NULL || pnanoseconds == NULL)
   {
      return -1;
   }

#if (TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
   {
      clock_serv_t cclock;
      mach_timespec_t mts;

      host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
      clock_get_time(cclock, &mts);
      mach_port_deallocate(mach_task_self(), cclock);

      *pseconds = mts.tv_sec;
      *pnanoseconds = mts.tv_nsec;
   }
#else
   clock_gettime(CLOCK_MONOTONIC, &t);

   *pseconds = t.tv_sec;
   *pnanoseconds = t.tv_nsec;
#endif

   return 0;
}

unsigned int pletoh32(unsigned char *pBuf, int BufLen)
{
   unsigned int iTmp = 0;
   int i = 0;
   for (i = (BufLen-1); i >= 0; i--)
   {
      iTmp = ((iTmp << 8) & 0xffffff00);
      iTmp = iTmp | pBuf[i];
   }
   return iTmp;
}

#ifdef __cplusplus
}
#endif

