#include <err.h>
#include <stdio.h>
#include <trusty_std.h>

#include <mtee_server.h>
#include <tz_cross/trustzone.h>
#include <tz_private/sys_ipc.h>
#include <tz_private/log.h>
#include <lib/mtee/mtee_srv.h>

//#define MTEE_API_LOG(fmt...) printf("[MTEE API]"fmt)
#define LOG_TAG "mtee_sync"
#define MTEE_THREAD_REE_SESSION 999 // current max session num is 512

#define NEED_DEBUG 0
#if NEED_DEBUG
#else
  #define DBG_LOG(fmt...)
#endif

int ioctl_send(const char *tag, enum MTEE_USER_CMD cmd, void *ptr)
{
	int rc;

	rc = ioctl(SYS_USER_MTEE_FD, cmd, ptr);
	if (rc!=NO_ERROR) {
		ERR_LOG("ioctl %s failed. rc = %d\n", tag, rc);
		return 0;
	}
	return 1;
}


/**
 * Mutex, create
 *
 * @return    A pointer to the the mutex. If fail, return 0.
 */
MTEE_MUTEX *MTEE_CreateMutex (void)
{
	user_mutex_t m_param;

	DBG_LOG("MUTEX CREATE\n");
	if (ioctl_send("MUTEX CREATE", MTEE_MUTEX_CREATE, &m_param))
		return (MTEE_MUTEX*)m_param.mutex;
	else
		return 0;
}

/**
 * Mutex, destroy
 *
 * @param mutex    A pointer to the mutex.
 * @return    0 if success. <0 for error.
 * 				-1: kernel space side error
 * 				-2: ioctl error
 */
int MTEE_DestoryMutex (MTEE_MUTEX *mutex)
{
	user_mutex_t m_param;

	DBG_LOG("MUTEX DESTROY\n");
	m_param.mutex = (uint32_t)mutex;
	if (ioctl_send("MUTEX DESTROY", MTEE_MUTEX_DESTROY, &m_param))
		return m_param.ret;
	else
		return -2;
}

/**
 * Mutex, lock
 *
 * @param mutex    A pointer to the mutex.
 */
int MTEE_LockMutex (MTEE_MUTEX *mutex)
{
	user_mutex_t m_param;

	DBG_LOG("MUTEX LOCK\n");
	m_param.mutex = (uint32_t)mutex;
	if (ioctl_send("MUTEX LOCK", MTEE_MUTEX_LOCK, &m_param))
		return m_param.ret;
	else
		return -2;
}

/**
 * Mutex, unlock
 *
 * @param mutex    A pointer to the mutex.
 */
int MTEE_UnlockMutex (MTEE_MUTEX *mutex)
{
	user_mutex_t m_param;

	DBG_LOG("MUTEX UNLOCK\n");
	m_param.mutex = (uint32_t)mutex;
	if (ioctl_send("MUTEX UNLOCK", MTEE_MUTEX_UNLOCK, &m_param))
		return m_param.ret;
	else
		return -2;
}

/**
 * Mutex, try lock
 *
 * @param mutex    A pointer to the mutex.
 * @return    1 if the mutex has been acquired successfully, and 0 on contention.
 * 				-1: kernel space side error
 * 				-2: ioctl error
 */
int MTEE_TryLockMutex (MTEE_MUTEX *mutex)
{
	user_mutex_t m_param;

	DBG_LOG("MUTEX TRYLOCK\n");
	m_param.mutex = (uint32_t)mutex;
	if (ioctl_send("MUTEX TRYLOCK", MTEE_MUTEX_TRYLOCK, &m_param))
		return m_param.ret;
	else
		return -2;
}

/**
 * Mutex, querey if it is locked or not
 *
 * @param mutex    A pointer to the mutex.
 * @return    1 if the mutex is locked, 0 if unlocked..
 * 				-1: ioctl error
 */
int MTEE_IsLockedMutex (MTEE_MUTEX *mutex)
{
	user_mutex_t m_param;

	DBG_LOG("MUTEX ISLOCK\n");
	m_param.mutex = (uint32_t)mutex;
	if (ioctl_send("MUTEX ISLOCK", MTEE_MUTEX_ISLOCK, &m_param))
		return m_param.ret;
	else
		return -1;
}

/* Semaphore
*/
/**
 * Semaphore, create
 *
 * @param val      initial value.
 * @return    A pointer to the the semaphore. If fail, return 0.
 */
MTEE_SEMAPHORE *MTEE_CreateSemaphore (int val)
{
	user_sema_t s_param;

	DBG_LOG("SEMAPHORE CREATE: val = %d\n", val);
    s_param.ret = val;
	if (ioctl_send("SEMA CREATE", MTEE_SEMA_CREATE, &s_param))
		return (MTEE_SEMAPHORE*)s_param.sema;
	else
		return 0;
}


/**
 * Semaphore, free
 *
 * @param semaphore    A pointer to the semaphore.
 * @return    0 if success. <0 for error.
 * 				-1: kernel space side error
 * 				-2: ioctl error
 */
int MTEE_DestroySemaphore (MTEE_SEMAPHORE *semaphore)
{
	user_sema_t s_param;

	DBG_LOG("SEMAPHORE DESTROY\n");
	s_param.sema = (uint32_t)semaphore;
	if (ioctl_send("SEMA DESTROY", MTEE_SEMA_DESTROY, &s_param))
		return s_param.ret;
	else
		return -2;
}

/**
 * Semaphore, down
 *
 * @param semaphore    A pointer to the semaphore.
 */
void MTEE_DownSemaphore (MTEE_SEMAPHORE *semaphore)
{
	user_sema_t s_param;

	DBG_LOG("SEMAPHORE DOWN\n");
	s_param.sema = (uint32_t)semaphore;
	ioctl_send("SEMA DOWN", MTEE_SEMA_DOWN, &s_param);
}

/**
 * Semaphore down, and can be interrupted by signal.
 *
 * @param semaphore    A pointer to the semaphore.
 * @return 0 if success. <0 for error.
 */
/*fix mtee sync*/
int MTEE_DownSemaphoreInterruptible (MTEE_SEMAPHORE *semaphore)
{
#if 0
    unsigned long *ptr;
    int *ret;

    ptr = MTEE_GetReeParamAddress ();
    ret = (int *)ptr;
    *ptr = (unsigned long)semaphore;
#if defined(__arm__) && defined(WITH_ARM_TRUSTED_FIRMWARE)
    *(ptr+1) = 0xffffffc0;
#endif
    MTEE_ReeServiceCall (REE_SERV_SEMAPHORE_DOWNINT);

    return *ret;
#endif

	print_mtee_orig_msg;
	return; //not support
}

/**
 * Semaphore, down with time-out
 *
 * @param semaphore    A pointer to the semaphore.
 * @param timeout    timeout value, in jiffies
 * @return    0 if the @timeout elapsed, and the remaining
 * jiffies if the condition evaluated to true before the timeout elapsed.
 * 				-1: kernel space side error
 * 				-2: ioctl error
 */
int MTEE_DownSemaphore_Timeout (MTEE_SEMAPHORE *semaphore, int timeout)
{
	user_sema_t s_param;

	DBG_LOG("SEMAPHORE DOWN_TIMEOUT\n");
	s_param.sema = (uint32_t)semaphore;
	s_param.timeout = timeout;
	if (ioctl_send("SEMA DOWN_TIMEOUT", MTEE_SEMA_DOWN_TIMEOUT, &s_param))
		return s_param.ret;
	else
		return -2;
}

/**
 * Semaphore, down try lock
 *
 * @param semaphore    A pointer to the semaphore.
 * @return    0 if semaphore is avalibale.
 * 				-1: kernel space side error
 * 				-2: ioctl error
 */
int MTEE_DownTryLockSemaphore (MTEE_SEMAPHORE *semaphore)
{
	user_sema_t s_param;

	DBG_LOG("SEMAPHORE TRYDOWN\n");
	s_param.sema = (uint32_t)semaphore;
	if (ioctl_send("SEMA TRYDOWN", MTEE_SEMA_TRYDOWN, &s_param))
		return s_param.ret;
	else
		return -2;
}

/**
 * Semaphore, up
 *
 * @param semaphore    A pointer to the semaphore.
 */
void MTEE_UpSemaphore (MTEE_SEMAPHORE *semaphore)
{
	user_sema_t s_param;

	DBG_LOG("SEMAPHORE UP\n");
	s_param.sema = (uint32_t)semaphore;
	ioctl_send("SEMA UP", MTEE_SEMA_UP, &s_param);
}

static TZ_RESULT ree_upsema(int session, uint64_t ksema_ptr, uint32_t cpu, uint32_t boost_enabled)
{
	TZ_RESULT ret;
	MTEEC_PARAM param[4];
	uint32_t paramTypes;

	paramTypes = TZ_ParamTypes2(TZPT_VALUE_OUTPUT, TZPT_VALUE_INPUT);
	param[0].value.a = ksema_ptr;
	param[0].value.b = (ksema_ptr >> 32);
	param[1].value.b = (cpu&0x0000ffff);
	// param[1].value.b b'15~b'0 use for cpu, b'31-b'16 use for boost_enable
	if (boost_enabled) param[1].value.b = param[1].value.b | 0x00010000;

	DBG_LOG("ree service call: ksema_ptr 0x%x%x, param1Valb=0x%x\n", param[0].value.b, param[0].value.a, param[1].value.b);
	ret = MTEE_ReeServiceCall(session, REE_SERVICE_CMD_KICK_SEM, paramTypes, param);
	if (ret)
		return TZ_RESULT_ERROR_COMMUNICATION;

	DBG_LOG("up sema ree return: %d\n", param[1].value.a);
	ret = param[1].value.a; //TODO: translate to TZ_RESULT
	return ret;
}

void MTEE_UpSemaphoreThread(MTEE_SEMAPHORE *semaphore, uint32_t cpu, uint32_t boost_enabled)
{
	int rc = -2;
	user_sema_t s_param;
	ioctl_send("REE ONGOING", MTEE_REE_ONGOING, &rc);

	if (likely(rc==0)) {
		DBG_LOG("cpu MTEE_UpSemaphore+\n");
		s_param.sema = (uint32_t)semaphore;
		ioctl_send("SEMA GET", MTEE_SEMA_GET, &s_param);
		DBG_LOG("after MTEE_SEMA_GET: ksema_ptr = 0x%llx \n", s_param.ksema_ptr);
		ree_upsema(MTEE_THREAD_REE_SESSION, s_param.ksema_ptr, cpu, boost_enabled);
		ioctl_send("REE ARRIVAL", MTEE_REE_ARRIVAL, NULL);
		DBG_LOG("cpu MTEE_UpSemaphore-\n");
	} else {
		DBG_LOG("normal MTEE_UpSemaphore\n");
		MTEE_UpSemaphore(semaphore);
	}
}

/* Event
*/
/**
 * Event, create
 *
 * @param signaled  Initial value for "signaled", true if != 0
 * @param flags     0 or MTEE_EVENT_AUTOUNSIGNAL
 * @return    A pointer to the event. If fail, return 0.
 */
MTEE_EVENT *MTEE_CreateEvent (int signaled, uint32_t flags)
{
	user_event_t param;

	param.status = signaled;
	param.timeout = flags;

	if (ioctl_send("EVENT NEW", MTEE_EVENT_CREATE, &param))
		return param.event;
	else
		return NULL;
}

/**
 * Event, free
 *
 * @param event    A pointer to the event.
 * @return    0 if success. <0 for error.
 * 				-1: kernel space side error
 * 				-2: ioctl error
 */
int MTEE_DestroyEvent (MTEE_EVENT *event)
{
	user_event_t param;

	if (!event) {
		ERR_LOG("destroy event: invalid event");
		return -2;
	}

	param.event = event;
	if (ioctl_send("EVENT DESTROY", MTEE_EVENT_DESTROY, &param))
		return param.status;
	else
		return -2;
}

/**
 * Wait event with timeout
 *
 * @param event    A pointer to the event.
 * @param timeout     timeout value in millisecond
 * @return     0 if success
 * 				-1: kernel space side error
 * 				-2: ioctl error
 */
int MTEE_WaitEvent_Timeout (MTEE_EVENT *event, int timeout)
{
	user_event_t param;

	if (!event) {
		ERR_LOG("wait event: invalid event");
		return -2;
	}

	param.event = event;
	param.timeout = timeout;

	if (ioctl_send("EVENT WAIT", MTEE_EVENT_WAIT, &param))
		return param.status;
	else
		return -2;
}

/**
 * Event, signal
 *
 * @param event    A pointer to the event.
 * @param resched   If != 0, waiting thread(s) are executed immediately.
 *                  Otherwise, waiting threads are placed at the end of the runqueue
 * @return      status, 0 if success.
 * 				-1: kernel space side error
 * 				-2: ioctl error
 */
int MTEE_SignalEvent (MTEE_EVENT *event, int reschedule)
{
	user_event_t param;

	if (!event) {
		ERR_LOG("wait event: invalid event");
		return -2;
	}

	param.event = event;
	param.status = reschedule;

	if (ioctl_send("EVENT SIGNAL", MTEE_EVENT_SIGNAL, &param))
		return param.status;
	else
		return -2;

}


/**
 * Event, unsignal
 *
 * @param event    A pointer to the event.
 * @return      status, 0 if success.
 * 				-1: kernel space side error
 * 				-2: ioctl error
 */
int MTEE_UnsignalEvent (MTEE_EVENT *event)
{
	user_event_t param;

	if (!event) {
		ERR_LOG("wait event: invalid event");
		return -2;
	}

	param.event = event;

	if (ioctl_send("EVENT SIGNAL", MTEE_EVENT_UNSIGNAL, &param))
		return param.status;
	else
		return -2;

}

