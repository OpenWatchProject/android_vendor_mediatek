#include "me_comm.h"
#include "me_result.h"
#include "me_device.h"

void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(lpCriticalSection, &mutexattr);
	pthread_mutexattr_destroy(&mutexattr);
}

void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	pthread_mutex_destroy(lpCriticalSection);
}

void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	pthread_mutex_lock(lpCriticalSection);
}

void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	pthread_mutex_unlock(lpCriticalSection);
}


BOOLEAN CloseHandle(OSAL_HANDLE hObject)
{
	if (!hObject)
		return OSAL_FALSE;

	int ret = 0;
	if (hObject->type & tagTHREAD)
	{
		ret = pthread_mutex_destroy(&hObject->mutex);
		if (ret < 0)
			perror("destroy thread's mutex error");
	}
	if (hObject->type & tagMUTEX)
	{
		ret = pthread_mutex_destroy(&hObject->mutex);
		if (ret < 0)
			perror("destroy mutex error");
	}
	if (hObject->type & tagCONDITION)
	{
		ret = pthread_cond_destroy(&hObject->condition);
		if (ret < 0)
			perror("destroy condition error");
	}

	free(hObject);
	return OSAL_TRUE;
};

OSAL_HANDLE CreateEvent(LPVOID attr, BOOLEAN bManualReset, BOOLEAN bInitialState, LPCTSTR lpName)
{
	OSAL_HANDLE hd = (OSAL_HANDLE)malloc(sizeof(HANDLE_CONTENT));
    if (attr == NULL || lpName == NULL)
    { // for warning
        ME_Print(ME_TAG "The parameter is NULL!\n") ;
	}
	if(hd == NULL)
		return NULL;
	hd->type = tagMUTEX|tagCONDITION;
	hd->signaled = bInitialState;
	hd->manualreset = bManualReset;
	pthread_mutex_init(&(hd->mutex), NULL);
	pthread_cond_init(&(hd->condition), NULL);

	return hd;
};

BOOLEAN SetEvent(OSAL_HANDLE hd)
{
	if (hd && (hd->type & tagCONDITION))
	{
		pthread_mutex_lock(&hd->mutex);
		if (hd->manualreset)
			pthread_cond_broadcast(&hd->condition);
		else
			pthread_cond_signal(&hd->condition);
		hd->signaled = OSAL_TRUE;
		pthread_mutex_unlock(&hd->mutex);

		return OSAL_TRUE;
	}
	return OSAL_FALSE;
}

BOOLEAN ResetEvent(OSAL_HANDLE hd)
{
	if (hd && (hd->type & tagCONDITION))
	{
		pthread_mutex_lock(&hd->mutex);
		hd->signaled = OSAL_FALSE;
		pthread_mutex_unlock(&hd->mutex);
		return OSAL_TRUE;
	}
	return OSAL_FALSE;
}

OSAL_HANDLE CreateMutex(LPVOID attr, BOOLEAN initialOwner, LPCTSTR lpName)
{
	OSAL_HANDLE hd = (OSAL_HANDLE)malloc(sizeof(HANDLE_CONTENT));
	if (attr == NULL || lpName == NULL || initialOwner == 0) {
	 //reserved parameter, for resolving warning
        ME_Print(ME_TAG "[Medrv] Warning: The parameter is NULL......");
	}


	if(hd == NULL)
		return NULL;
	hd->type = tagMUTEX;
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&hd->mutex, &mutexattr);
	pthread_mutexattr_destroy(&mutexattr);
	return hd;
}

BOOLEAN ReleaseMutex(OSAL_HANDLE hd)
{
	if (hd && (hd->type == tagMUTEX))
	{
		pthread_mutex_unlock(&hd->mutex);
		return OSAL_TRUE;
	}
	return OSAL_FALSE;
}

void* pthreadHelper(void* arg)
{
	winThreadFuncWrapper* wp = (winThreadFuncWrapper*)arg;
	DWORD dwRet = (*wp->fn)(wp->arg);
	ME_Print(ME_TAG "dwRet = %d!\n", dwRet);
	*wp->psig = OSAL_TRUE;
	free(wp);
	return NULL;
}

OSAL_HANDLE _beginthreadex(LPVOID attr, DWORD stack_size, winThreadFunc wfn,
							 LPVOID arglist, DWORD initflag, DWORD* thrdaddr)
{
	if (attr || stack_size || initflag)
		printf("some args unsupported in _beginthreadex wrapper~~~\n");

	OSAL_HANDLE hd = (OSAL_HANDLE)malloc(sizeof(HANDLE_CONTENT));
	if(hd == NULL)
		return NULL;
	hd->type = tagTHREAD;
	hd->signaled = OSAL_FALSE;

	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&hd->mutex, &mutexattr);
	pthread_mutexattr_destroy(&mutexattr);
	winThreadFuncWrapper* wp = (winThreadFuncWrapper*)malloc(sizeof(winThreadFuncWrapper));
        if(wp == NULL)
        {
            free(hd);
	    return NULL;
        }
	wp->fn = wfn;
	wp->arg = arglist;
	wp->psig = &hd->signaled;
	int tid = pthread_create(&hd->thread, NULL, pthreadHelper, wp);
	if (thrdaddr)
		*thrdaddr = tid;
	return hd;
}
/*
BOOLEAN TerminateThread(OSAL_HANDLE hd, DWORD dwExitCode)
{
	if (hd && (hd->type == tagTHREAD))
	{
		pthread_cancel(hd->thread);
		return true;
	}
	return false;
}
*/
DWORD WaitForSingleObject(OSAL_HANDLE hd, DWORD dwMilliseconds)
{
	//	should the EINVAL be considered?
	DWORD dwRet = WAIT_OBJECT_0;

	if (hd && (hd->type & tagCONDITION))
	{
		pthread_mutex_lock(&hd->mutex);

		if (hd->signaled)	//	signaled before
			dwRet = WAIT_OBJECT_0;
		else if (dwMilliseconds == INFINITE)
			pthread_cond_wait(&hd->condition, &hd->mutex);
		else if (dwMilliseconds == 0)
			dwRet = WAIT_TIMEOUT;
		else
		{
			struct timespec ts;
			struct timeval	tv;
			gettimeofday(&tv, NULL);
			ts.tv_sec = tv.tv_sec + dwMilliseconds/1000;
			ts.tv_nsec = tv.tv_usec*1000 + ((double)(dwMilliseconds%1000))*1000000000;
			int err = pthread_cond_timedwait(&hd->condition, &hd->mutex, &ts);
			if (err == ETIMEDOUT)
				dwRet = WAIT_TIMEOUT;
		}

		if (hd->manualreset && dwRet == WAIT_OBJECT_0)
			hd->signaled = OSAL_TRUE;
		else
			hd->signaled = OSAL_FALSE;

		pthread_mutex_unlock(&hd->mutex);
	}
	else if (hd && (hd->type & tagMUTEX))
	{
		if (dwMilliseconds == INFINITE)
			pthread_mutex_lock(&hd->mutex);
		else if (dwMilliseconds == 0)
		{
			int err = pthread_mutex_trylock(&hd->mutex);
			if (err == EBUSY)
				dwRet = WAIT_TIMEOUT;
		}
		else	//	not support wait a mutex with timeout
			dwRet = WAIT_TIMEOUT;
	}
	else if (hd && (hd->type & tagTHREAD))
	{
		LPVOID ret;
		if (dwMilliseconds == INFINITE)
		{
			pthread_mutex_lock(&hd->mutex);
			pthread_join(hd->thread, &ret);
			pthread_mutex_unlock(&hd->mutex);
		}
		else if (dwMilliseconds == 0)
			dwRet = hd->signaled?WAIT_OBJECT_0:WAIT_TIMEOUT;
		else
		{
			//	cuz pthread_join doesnt support timeout, i have to
			//	simulate it with nanosleep..
			//	someone say the accurate of nanosleep is 10ms, so..
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 10*1000000;	//	10ms
			while (!hd->signaled && dwMilliseconds > 10)
			{
				nanosleep(&ts, NULL);
				dwMilliseconds -= 10;
			}
			dwRet = hd->signaled?WAIT_OBJECT_0:WAIT_TIMEOUT;
		}
	}

	return dwRet;
}

DWORD WaitForMultipleObjects(DWORD nCount, const OSAL_HANDLE* lpHandles, 
									BOOLEAN bWaitAll, DWORD dwMilliseconds)
{
	DWORD dwRet = WAIT_FAILED;

	if (lpHandles == NULL)
		return dwRet;

	unsigned int i;
	DWORD ret = WAIT_OBJECT_0;
	if (bWaitAll)
	{
		while (dwMilliseconds > 10)
		{
			for (i = 0; i < nCount; i ++)
			{
				ret = WaitForSingleObject(lpHandles[i], 0);
				if (ret != WAIT_OBJECT_0)
					break;
			}

			if (i == nCount && ret == WAIT_OBJECT_0)
			{
				dwRet = WAIT_OBJECT_0;
				break;
			}
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 10*1000000;	//	10ms
			nanosleep(&ts, NULL);
			dwMilliseconds -= 10;
		}
	}
	else
	{
		while (dwMilliseconds > 10)
		{
			for (i = 0; i < nCount; i ++)
			{
				ret = WaitForSingleObject(lpHandles[i], 0);
				if (ret == WAIT_OBJECT_0)
					break;
			}
			if (ret == WAIT_OBJECT_0)
			{
				dwRet = WAIT_OBJECT_0 + i;
				break;
			}
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 10*1000000;	//	10ms
			nanosleep(&ts, NULL);
			dwMilliseconds -= 10;
		}
	}

	if (dwMilliseconds <= 10)
		dwRet = WAIT_TIMEOUT;

	return dwRet;
}

void *me_memset(void *dst, int val, int count)
{
	void *start = dst;

	while (count--) {
		*(char *)dst = (char)val;
		dst = (char *)dst + 1;
	}
	return(start);
}

void *me_memcpy(void *dst, const void *src, unsigned int count)
{
	void * ret = dst;

	while (count--)
	{
		*(char *)dst = *(char *)src;
		dst = (char *)dst + 1;
		src = (char *)src + 1;
	}

	return ret;
}

unsigned int me_strlen(const char *str)
{
	const char *eos = str;

	while( *eos++ );

	return( (int)(eos - str - 1) );
}

int me_memcmp(const void *buf1, const void *buf2, unsigned int count)
{
	if (!count)
		return(0);

	while ( --count && *(char *)buf1 == *(char *)buf2 ) 
	{
		buf1 = (char *)buf1 + 1;
		buf2 = (char *)buf2 + 1;
	}

	return( *((unsigned char *)buf1) - *((unsigned char *)buf2) );

}

char *me_strcpy(char *dst, const char *src )
{
	char * cp = dst;

	while((*cp++ = *src++) != 0);


	return dst;
}

char * me_strstr (const char * str1, const char * str2)
{
	char *cp = (char *) str1;
	char *s1, *s2;

	if ( !*str2 )
		return((char *)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (char *) str2;

		while ( *s1 && *s2 && !(*s1-*s2) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}
	
	return NULL;
}

int me_atoi(const char *str)
{
	int c;
	int total;
	int sign;

	while (*str == ' ')
		++str;

	c = (int)(unsigned char)*str++;
	sign = c;
	if (c == '-' || c == '+')
		c = (int)(unsigned char)*str++;

	total = 0;

	while((c >='0')&&(c <= '9'))
	{
		total = 10 * total + (c - '0');
		c = (int)(unsigned char)*str++;
	}

	if (sign == '-')
		return (int)-total;
	else
		return (int)total;
}

void *me_malloc(unsigned int size)
{
	return malloc(size);
}

void me_free(void *pData)
{
	free(pData);
}

void me_initlock(OSALLOCK *pLock)
{
	InitializeCriticalSection(&pLock->Lock);
}

void me_lock(OSALLOCK *pLock)
{
	EnterCriticalSection(&pLock->Lock);
}

void me_unlock(OSALLOCK *pLock)
{
	LeaveCriticalSection(&pLock->Lock);
}

unsigned int __stdcall TimerThreadFunc(LPVOID param)
{
	unsigned int uTimeOut = INFINITE;
	OSALTIMER *pTimer = (OSALTIMER *)(param);

	const OSAL_HANDLE hEvents[] =
	{
		pTimer->hSetEvt,
		pTimer->hKillEvt,
		pTimer->hExitEvt
	};

	ResetEvent(hEvents[0]);
	ResetEvent(hEvents[1]);
	ResetEvent(hEvents[2]);

	while(OSAL_TRUE)
	{
		switch(WaitForMultipleObjects(3, hEvents, OSAL_FALSE, uTimeOut))
		{
			case WAIT_OBJECT_0:	//set event
				uTimeOut = pTimer->duetime;
				ME_Print(ME_TAG "[Medrv] [Timer thread] set event");
				break;
			case WAIT_OBJECT_0+1:  //kill event
				ME_Print(ME_TAG "[Medrv] [Timer thread] Kill event");
				uTimeOut = INFINITE;
				SetEvent(pTimer->hKilledEvt);
				break;
			case WAIT_OBJECT_0+2:  //exit event
				ME_Print(ME_TAG "[Medrv] [Timer thread] exit event");
				return 0;
			case WAIT_TIMEOUT:
				uTimeOut = INFINITE;
				ME_Print(ME_TAG "[Medrv] [Timer thread] Time out call back - begin");
				SetEvent(pTimer->hKilledEvt);
				pTimer->timer_cb(pTimer->pData);
				ResetEvent(pTimer->hKilledEvt);
				ME_Print(ME_TAG "[Medrv] [Timer thread] Time out call back - end");
				break;
			default:
				break;
		}
	}

	return 0;
}

unsigned int StartTimer(OSALTIMER *timer)
{
	if (NULL != timer->hThread)
		return 0;
	if (NULL == timer->hSetEvt)
		timer->hSetEvt = CreateEvent(NULL, OSAL_FALSE, OSAL_FALSE, NULL);
	if (NULL == timer->hKillEvt)
		timer->hKillEvt = CreateEvent(NULL, OSAL_FALSE, OSAL_FALSE, NULL);
	if (NULL == timer->hExitEvt)
		timer->hExitEvt = CreateEvent(NULL, OSAL_FALSE, OSAL_FALSE, NULL);
	if (NULL == timer->hKilledEvt)
		timer->hKilledEvt = CreateEvent(NULL, OSAL_FALSE, OSAL_FALSE, NULL);


	timer->hThread = (OSAL_HANDLE)_beginthreadex(
	NULL, 0, TimerThreadFunc, (LPVOID)timer, 0, NULL);

	Sleep(1000);
	return (NULL != timer->hThread);
}


void me_inittimer(OSALTIMER *timer)
{
	timer->hThread = NULL;
	timer->hSetEvt = NULL;
	timer->hKillEvt = NULL;
	timer->hExitEvt = NULL;
	timer->hKilledEvt = NULL;
	timer->duetime =  INFINITE;
	StartTimer(timer);
}

void me_settimer(OSALTIMER *timer, int duetime)
{
	timer->duetime = duetime;
	SetEvent(timer->hSetEvt);
}

void me_killtimer(OSALTIMER *timer, int event)
{
	if(event == AT_TIMEOUT)
	{
		ME_Print(ME_TAG "[Medrv] Kill timer, event is AT_TIMEOUT");
		return;
	}

	SetEvent(timer->hKillEvt);
	WaitForSingleObject(timer->hKilledEvt, INFINITE);
	ME_Print(ME_TAG "[Medrv] Killed timer");
}

void me_exittimer(OSALTIMER *timer)
{
	SetEvent(timer->hExitEvt);
	WaitForSingleObject(timer->hThread, INFINITE);
	CloseHandle(timer->hThread);
	timer->hThread = NULL;
	ME_Print(ME_TAG "[Medrv] Exit timer thread");

}

NTSTATUS me_send_at(void* ioproxy, const char *str, ME_TX_NOTIFY notify, void *pData)
{
	NTSTATUS ret = STATUS_FAILED;
	
	if(ioproxy != NULL)
	{
		CDevice *pDev = (CDevice*)ioproxy;
		if(pDev->WriteData((const char*)str, (const int)me_strlen(str)))
		{
			notify(STATUS_SUCCESS, (const int)me_strlen(str), pData);
			ret = STATUS_SUCCESS;
		}
		else
		{
			notify(STATUS_FAILED, (const int)me_strlen(str), pData);
		}

	}

	PMEFSM pfsm = (PMEFSM)pData;

	WaitForSingleObject(pfsm->req.hBlockEvt, INFINITE);
	ResetEvent(pfsm->req.hBlockEvt);

	return (ret == STATUS_SUCCESS);
}

void me_respond_ready(void *pData)
{
	PMEFSM pfsm = (PMEFSM)pData;
	SetEvent(pfsm->req.hBlockEvt);
	ME_Print(ME_TAG "[Medrv] Reply......");

}

NTSTATUS me_get_rettype(void *pData)
{
	ATResult *pRet = (ATResult*)((PMEFSM)pData)->req.context;
	return pRet->retType;
}
