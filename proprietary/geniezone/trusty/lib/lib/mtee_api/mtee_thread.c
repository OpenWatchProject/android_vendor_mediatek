#include <stdio.h>
#include <stdlib.h>
#include <trusty_std.h>

#include <mtee_server.h>
#include <tz_cross/trustzone.h>
#include <tz_private/sys_ipc.h>
#include <tz_private/log.h>

#include <err.h>
#include <lib/mtee/mtee_srv.h>
#include <assert.h>

#define LOG_TAG "mtee_th"


#ifdef __NEBULA_HEE__

#include <pthread.h>

TZ_RESULT MTEE_CreateGZThread(MTEE_THREAD_HANDLE *thread, MTEE_ThreadFunc func,
		void *args, uint32_t priority)
{
	int ret;

	ret = pthread_create((pthread_t *)thread, NULL, func, args);
	if(ret != 0) {
		DBG_LOG("[ERROR] create thread fail, ret = %d\n", ret);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_JoinThread(MTEE_THREAD_HANDLE thread)
{
	int ret;
	void *retval;

	ret = pthread_join((pthread_t)thread, &retval);
	if(ret != 0) {
		DBG_LOG("[ERROR] join thread fail, ret = %d\n", ret);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}


void mtee_exit(int status)
{
	pthread_exit((void *)&status);
}

#else

#define MTEE_THREAD_REE_SESSION 999 // current max session num is 512

TZ_RESULT MTEE_CreateGZThread(MTEE_THREAD_HANDLE *thread, MTEE_ThreadFunc func,
		void *args, uint32_t priority)
{
	long ret;
	if (!thread)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	ret = user_thread_create(thread, func, args, priority);
	if (ret < 0)
		return TZ_RESULT_ERROR_GENERIC;

	ret = user_thread_resume(thread);
	if (ret < 0)
		return TZ_RESULT_ERROR_GENERIC;

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_JoinThread(MTEE_THREAD_HANDLE thread)
{
	long ret;
	if (!thread)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	ret = user_thread_join(&thread, INFINITE_TIME);
	if (ret < 0)
		return TZ_RESULT_ERROR_GENERIC;

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_JoinThreadTimeout(MTEE_THREAD_HANDLE thread, uint32_t timeout)
{
	long ret;
	if (!thread)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	ret = user_thread_join(&thread, timeout);
	if (ret < 0)
		return TZ_RESULT_ERROR_GENERIC;

	return TZ_RESULT_SUCCESS;
}

static int fork_threads(MTEE_THREAD_HANDLE *th,
		MTEE_ThreadFunc func, void *args, uint32_t priority)
{
	int ret;

	ret = user_thread_create(th, func, args, priority);
	if (ret < 0)
		return ret;

	return 0;
}

static TZ_RESULT ree_fork(int session, MTEE_THREAD_HANDLE thread, uint32_t cpu, uint32_t boost_enabled)
{
	TZ_RESULT ret;
	MTEEC_PARAM param[4];
	uint32_t paramTypes;

	paramTypes = TZ_ParamTypes2(TZPT_VALUE_OUTPUT, TZPT_VALUE_INPUT);

	param[0].value.a = thread;
	param[0].value.b = (thread >> 32);
	param[1].value.b = (cpu&0x0000ffff);
	// param[1].value.b b'15~b'0 use for cpu, b'31-b'16 use for boost_enable
	if (boost_enabled) param[1].value.b = param[1].value.b | 0x00010000;

#ifdef GZ_MULTI_THREAD_DEBUG
	DBG_LOG("ree service call: thread 0x%x%x, param1Valb=0x%x\n", param[0].value.b, param[0].value.a, param[1].value.b);
#endif
	ret = MTEE_ReeServiceCall(session, REE_SERVICE_CMD_NEW_THREAD, paramTypes, param);
	if (ret)
		return TZ_RESULT_ERROR_COMMUNICATION;

#ifdef GZ_MULTI_THREAD_DEBUG
	DBG_LOG("ree return: %d\n", param[1].value.a);
#endif
	ret = param[1].value.a; //TODO: translate to TZ_RESULT

	return ret;
}

TZ_RESULT __MTEE_CreateThread(MTEE_THREAD_HANDLE *handle, MTEE_ThreadFunc func,
		void *args, uint32_t priority, uint32_t cpu, uint32_t boost_enabled)
{
	TZ_RESULT ret;

	if (!handle)
		return TZ_RESULT_ERROR_BAD_PARAMETERS;

	ret = fork_threads(handle, func, args, priority);
	if (ret != TZ_RESULT_SUCCESS) return ret;
	return ree_fork(MTEE_THREAD_REE_SESSION, *handle, cpu, boost_enabled);
}

TZ_RESULT MTEE_CreateThread(MTEE_THREAD_HANDLE *handle, MTEE_ThreadFunc func,
		void *args, uint32_t priority)
{
	TZ_RESULT ret;
	int rc = 0;

	ret = ioctl(SYS_USER_MTEE_FD, MTEE_REE_ONGOING, &rc);
	assert(!ret);
	if (likely(rc==0)) {
		ret = __MTEE_CreateThread(handle, func, args, priority, 0, 0);
		assert(!ret);
		ret = ioctl(SYS_USER_MTEE_FD, MTEE_REE_ARRIVAL, NULL);
		assert(!ret);
	} else {
		ret = MTEE_CreateGZThread(handle, func, args, priority);
	}
	return ret;
}

TZ_RESULT MTEE_CreateThreadWithCPU(MTEE_THREAD_HANDLE *handle, MTEE_ThreadFunc func,
        void *args, uint32_t priority, uint32_t cpu, uint32_t boost_enabled)
{
	TZ_RESULT ret;
	int rc = 0;

	ret = ioctl(SYS_USER_MTEE_FD, MTEE_REE_ONGOING, &rc);
	assert(!ret);
	if (likely(rc==0)) {
		ret = __MTEE_CreateThread(handle, func, args, priority, cpu, boost_enabled);
		assert(!ret);
		ret = ioctl(SYS_USER_MTEE_FD, MTEE_REE_ARRIVAL, NULL);
		assert(!ret);
	} else {
		ret = MTEE_CreateGZThread(handle, func, args, priority);
	}
	return ret;
}

TZ_RESULT MTEE_UserThreadStackUsed(uint32_t *stack_used)
{
	int rc;
	user_thread_t param;
	uint32_t sp_val = 0;

	rc = ioctl(SYS_USER_MTEE_FD, MTEE_THREAD_STACK_START, &param);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl MTEE_THREAD_STACK_START failed. rc = %d\n", rc);
		return TZ_RESULT_ERROR_GENERIC;
	}

	__asm__ volatile ("mov %0, sp\n" : "=r" (sp_val) );
	*stack_used = param.trusty_thread_stack_start - sp_val;

	DBG_LOG("%s: stack start = 0x%x, sp = 0x%x, stack_used = %u \n", __func__, param.trusty_thread_stack_start, sp_val, *stack_used);

	return TZ_RESULT_SUCCESS;
}

#endif
