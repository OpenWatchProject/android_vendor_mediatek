#ifndef _ME_OSAL_H
#define _ME_OSAL_H

#include <cutils/log.h>
#include "Win2L.h"

#define ME_TAG			"[Factory-Mode]"
#define ME_Print 		ALOGD
#define NTSTATUS		int
#define PUCHAR			char*
#define ULONG_PTR		unsigned int
#define ME_ULONG            unsigned int
#define STATUS_SUCCESS	0x00000001L
#define STATUS_FAILED	0x00000000L
#define me_sprintf      sprintf

#define ME_HANDLE		void*
#define URC_COUNT		5
#define RAW_DATA_LEN	1024*8
#define CTL_DATA_LEN	1024*8
#define AT_CMD_LEN		256
#define	EXP_CMD_LEN		16
#define INVALID_PARA	-1

#define STATUS_INVALID_PARAMETER    100 //ER_UNKNOWN
#define SHORT_TIMEOUT_ELAPSE		3*1000
#define MIDDLE_TIMEOUT_ELAPSE            60*1000
#define LONG_TIMEOUT_ELAPSE			180*1000

typedef enum urc_internal_t
{
	ID_ECSQ  = 0,	
	ID_CREG,	
	ID_CGREG,	
	ID_PSBEARER,
	ID_EIND,	
	ID_RING,	
	ID_ESIMS,	
	ID_ECPI,	
	ID_ESPEECH,
	ID_CONN,
	ID_VPUP,
	ID_VPON,
	ID_EUSIM,
  ID_UIMST, 
	ID_MAX   /*10*/

}URC_ID;

typedef NTSTATUS (*ME_TX_NOTIFY)(NTSTATUS status, int len, void *pData);
typedef void (*ME_ATECallback) (const char *pData, int len);
typedef void (*ME_Callback) (void *pData);

typedef struct tagOSALLOCK
{
	CRITICAL_SECTION	Lock;
}OSALLOCK;

typedef struct tagOSALTIMER
{
	void	(*timer_cb)(void*);
	void	*pData;

	OSAL_HANDLE hThread;
	OSAL_HANDLE hSetEvt;
	OSAL_HANDLE hKillEvt;
	OSAL_HANDLE hExitEvt;
	OSAL_HANDLE hKilledEvt;
	unsigned int   duetime;

}OSALTIMER;

OSAL_HANDLE CreateEvent(LPVOID attr, BOOLEAN bManualReset, BOOLEAN bInitialState, LPCTSTR lpName);
BOOLEAN SetEvent(OSAL_HANDLE hd);
BOOLEAN ResetEvent(OSAL_HANDLE hd);
BOOLEAN CloseHandle(OSAL_HANDLE hObject);
OSAL_HANDLE CreateMutex(LPVOID attr, BOOLEAN initialOwner, LPCTSTR lpName);
BOOLEAN ReleaseMutex(OSAL_HANDLE hd);
OSAL_HANDLE _beginthreadex(LPVOID attr, DWORD stack_size, winThreadFunc wfn, 
							 LPVOID arglist, DWORD initflag, DWORD* thrdaddr);
DWORD WaitForSingleObject(OSAL_HANDLE hd, DWORD dwMilliseconds);

void *me_memset(void *dst, int val, int count);
void *me_malloc(unsigned int size);
void me_free(void *pData);
char *me_strcpy(char *dst, const char *src);
void *me_memcpy( void *dst, const void *src, unsigned int count);
unsigned int me_strlen(const char *str);
int me_memcmp(const void *buf1, const void *buf2, unsigned int count);
int me_atoi(const char *str);
char * me_strstr (const char * str1, const char * str2);
void me_initlock(OSALLOCK *pLock);
void me_lock(OSALLOCK *pLock);
void me_unlock(OSALLOCK *pLock);
void me_inittimer(OSALTIMER *timer);
void me_settimer(OSALTIMER *timer, int duetime);
void me_killtimer(OSALTIMER *timer, int event);
void me_exittimer(OSALTIMER *timer);
void me_respond_ready(void *pData);
NTSTATUS me_get_rettype(void *pData);
NTSTATUS me_send_at(void* ioproxy, const char *str, ME_TX_NOTIFY notify, void *pData);
NTSTATUS me_send_pdu(void* ioproxy, const char *str, ME_TX_NOTIFY notify, void *pData);

#endif //_ME_OSAL_H
