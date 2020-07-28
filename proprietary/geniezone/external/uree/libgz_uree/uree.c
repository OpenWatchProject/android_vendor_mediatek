#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <tz_cross/trustzone.h>
#include <tz_cross/ta_mem.h>
#include <uree/system.h>
#include <uree/mem.h>
#include <kree/tz_mod.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <log/log.h>
#include <tz_cross/tz_error_strings.h>


/* GZ Ree service
*/
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "gz_uree"
#endif

#define GZ_DEV_NAME "/dev/gz_kree"

#define SUPPORT_TAG_API 0
#define SUPPORT_READONLY_API 0

pthread_mutex_t uree_init_mutex = PTHREAD_MUTEX_INITIALIZER;
static int gz_fd = -1;

static TZ_RESULT UREE_OpenGzFd(void) {
	int fd;
	int ret = TZ_RESULT_SUCCESS;

	pthread_mutex_lock(&uree_init_mutex);
	if (gz_fd >= 0)
		goto out;

	// Open GZ device.
	fd = open(GZ_DEV_NAME, O_RDWR);
	if (fd < 0) {
		ALOGE("%s open fd fail, err = %s\n", __FUNCTION__, strerror(errno));
		ret = TZ_RESULT_ERROR_GENERIC;
	}
	else
		gz_fd = fd;

out:
	pthread_mutex_unlock(&uree_init_mutex);
	return ret;
}

static TZ_RESULT __UREE_CreateSession(const char *ta_uuid,
	UREE_SESSION_HANDLE *pHandle, const char *tag)
{
	TZ_RESULT ret;
	struct kree_session_cmd_param param = {0};
	int iret;

	if (gz_fd < 0) {
		ret = UREE_OpenGzFd();
		if (ret != TZ_RESULT_SUCCESS)
			return ret;
    }

	if (!pHandle || !ta_uuid) {
		ALOGE("%s:!pHandle || !ta_uuid\n", __func__);
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	param.data = (uint64_t)(unsigned long)ta_uuid;
	if (tag == NULL) {
		iret = ioctl(gz_fd, MTEE_CMD_OPEN_SESSION, &param);
	} else {
		#if SUPPORT_TAG_API
		param.tag = (uint64_t)(unsigned long)tag;
		iret = ioctl(gz_fd, MTEE_CMD_OPEN_SESSION_WITH_TAG, &param);
		#else
		iret = TZ_RESULT_ERROR_ITEM_NOT_FOUND;
		#endif
    }

	*pHandle = param.handle;

	if (iret) {
		ALOGE("%s: ioctl ret %d, err %d (%s))\n", __FUNCTION__, iret, errno,
			strerror(errno));
		return TZ_RESULT_ERROR_GENERIC;
	}

	if (param.ret != TZ_RESULT_SUCCESS)
		return param.ret;

	//*pHandle = param.handle;
	return TZ_RESULT_SUCCESS;
}

TZ_RESULT UREE_CreateSession(const char *ta_uuid,
	UREE_SESSION_HANDLE *pHandle)
{
    return __UREE_CreateSession(ta_uuid, pHandle, NULL);
}

TZ_RESULT UREE_CreateSessionWithTag(const char *ta_uuid,
	UREE_SESSION_HANDLE *pHandle, const char *tag)
{
    return __UREE_CreateSession(ta_uuid, pHandle, tag);
}

TZ_RESULT UREE_CloseSession(UREE_SESSION_HANDLE handle)
{
	struct kree_session_cmd_param param;
	int iret;

	if (gz_fd < 0) {
		ALOGE("%s: invalid fd %d\n", __FUNCTION__, gz_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	param.handle = handle;
	iret = ioctl(gz_fd, MTEE_CMD_CLOSE_SESSION, &param);
	if (iret) {
		ALOGE("%s: ioctl ret %d, err %d (%s))\n", __FUNCTION__, iret, errno,
			strerror(errno) );
		return TZ_RESULT_ERROR_GENERIC;
	}

	return param.ret;
}

TZ_RESULT UREE_TeeServiceCall(UREE_SESSION_HANDLE handle, uint32_t command,
	uint32_t paramTypes, MTEEC_PARAM param[4])
{
	struct kree_tee_service_cmd_param cparam;
	int iret;

	if (gz_fd < 0) {
		ALOGE("%s invalid fd %d\n", __FUNCTION__, gz_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	cparam.handle = handle;
	cparam.command = command;
	cparam.paramTypes = paramTypes;
	cparam.param = (uint64_t)(unsigned long)param;

	iret = ioctl(gz_fd, MTEE_CMD_TEE_SERVICE, &cparam);
	if (iret) {
		ALOGE("%s: ioctl ret %d, err %d (%s))\n", __FUNCTION__, iret, errno,
			strerror(errno) );
		return TZ_RESULT_ERROR_GENERIC;
	}

	return cparam.ret;
}

const char *TZ_GetErrorString(TZ_RESULT res)
{
	return _TZ_GetErrorString(res);
}

struct user_shm_param {
	uint32_t session;
	uint32_t shm_handle;
	UREE_SHAREDMEM_PARAM_US param;
};

#define voidp_2_uint32(pointer) ((uint32_t) ((0x00000000ffffffff) & *((uint64_t *) &pointer)))
#define voidp_2_uint64(pointer) (0xffffffffffffffff) & *((uint64_t *) &pointer)

#define voidpointer_2_uint64(data, pointer)	\
    if (sizeof(void*) == 4) {	\
        data = (uint64_t) voidp_2_uint32 (pointer);	\
    } else {	\
        data = (uint64_t) voidp_2_uint64 (pointer);	\
    }

static TZ_RESULT __UREE_RegisterSharedmem(UREE_SESSION_HANDLE handle,
	UREE_SHAREDMEM_HANDLE *shm_handle, UREE_SHAREDMEM_PARAM *param,
	const char *tag, uint32_t readOnly)
{
	struct user_shm_param p;
	int iret;

	ALOGI("UREE register shared mem\n");

	if ((param->buffer == NULL) || (param->size == 0)) {
		ALOGE ("%s: input parameter error! stop!\n", __func__);
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	if (gz_fd < 0) {
		ALOGE("%s invalid fd %d\n", __FUNCTION__, gz_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	*shm_handle = 0; /* 0: invalid handle */

	/* register shared memory */
	p.session = (uint32_t)handle;
	p.shm_handle = 0;
	p.param.size = param->size;
	p.param.region_id = param->region_id;
	voidpointer_2_uint64 (p.param.buffer, param->buffer);
#if SUPPORT_READONLY_API
	p.control = readOnly;
#endif

	ALOGI ("[%s] ====> sizeof(p)=%x  \n", __FUNCTION__, (uint32_t) sizeof(p));
	ALOGI ("[%s] ====> p.param.size=%x, param->buffer=0x%x  \n", __func__,
		p.param.size, (uint32_t) p.param.buffer);

	if (tag == NULL) {
		iret = ioctl(gz_fd, MTEE_CMD_SHM_REG, &p);
	} else {
#if SUPPORT_TAG_API
		iret = ioctl(gz_fd, MTEE_CMD_SHM_REG_WITH_TAG, &p);
#else
		iret = TZ_RESULT_ERROR_ITEM_NOT_FOUND;
#endif
	}
	if (iret) {
		ALOGE("%s: ioctl ret %d, err %d (%s))\n", __FUNCTION__, iret, errno,
			strerror(errno) );
		return TZ_RESULT_ERROR_GENERIC;
	}

	*shm_handle = p.shm_handle;
	if (p.shm_handle == 0) {
		ALOGE("%s shm hd = %d (fail)\n", __func__, p.shm_handle);
		return TZ_RESULT_ERROR_GENERIC;
	}
	return TZ_RESULT_SUCCESS;
}

TZ_RESULT UREE_RegisterSharedmem(UREE_SESSION_HANDLE handle,
	UREE_SHAREDMEM_HANDLE *shm_handle, UREE_SHAREDMEM_PARAM *param)
{
	return __UREE_RegisterSharedmem(handle, shm_handle, param, NULL, 0);
}

TZ_RESULT UREE_RegisterSharedmemWithTag(UREE_SESSION_HANDLE handle,
	UREE_SHAREDMEM_HANDLE *shm_handle, UREE_SHAREDMEM_PARAM *param,
	const char *tag)
{
	return __UREE_RegisterSharedmem(handle, shm_handle, param, tag, 0);
}

TZ_RESULT UREE_RegisterSharedmem_ReadOnly(UREE_SESSION_HANDLE handle,
	UREE_SHAREDMEM_HANDLE *shm_handle, UREE_SHAREDMEM_PARAM *param)
{
	return __UREE_RegisterSharedmem(handle, shm_handle, param, NULL, 1);
}

TZ_RESULT UREE_RegisterSharedmem_ReadOnlyWithTag(
	UREE_SESSION_HANDLE handle,	UREE_SHAREDMEM_HANDLE *shm_handle,
	UREE_SHAREDMEM_PARAM *param, const char *tag)
{
	return __UREE_RegisterSharedmem(handle, shm_handle, param, tag, 1);
}


TZ_RESULT __UREE_UnregisterSharedmem(UREE_SESSION_HANDLE handle,
	UREE_SHAREDMEM_HANDLE shm_handle, uint32_t read_only)
{
	struct user_shm_param p;
	int iret;

	if (gz_fd < 0) {
		ALOGE("%s invalid fd %d\n", __FUNCTION__, gz_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	p.session = (uint32_t)handle;
	p.param.buffer = (uint64_t)(unsigned long)NULL;
	p.param.size = 0;
	p.shm_handle = shm_handle;
#if SUPPORT_READONLY_API
	p.control = read_only;  // for write
#endif
	iret = ioctl(gz_fd, MTEE_CMD_SHM_UNREG, &p);

	if (iret) {
		ALOGE("%s: ioctl ret %d, err %d (%s))\n", __FUNCTION__, iret, errno,
			strerror(errno) );
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT UREE_UnregisterSharedmem(UREE_SESSION_HANDLE handle,
	UREE_SHAREDMEM_HANDLE shm_handle)
{
    /* MTEE_CMD_SHM_UNREG not yet implemented, so use UREE_TeeServiceCall */
    // return __UREE_UnregisterSharedmem(handle, shm_handle, 0);

	MTEEC_PARAM p[4];
	TZ_RESULT ret;

	p[0].value.a = (uint32_t) shm_handle;
	ret = UREE_TeeServiceCall(handle, TZCMD_MEM_SHAREDMEM_UNREG,
		TZ_ParamTypes1(TZPT_VALUE_INPUT), p);
	if (ret!= TZ_RESULT_SUCCESS) {
		ALOGE ("[%s] ====> cmd:TZCMD_MEM_SHAREDMEM_UNREG ret Fail(0x%x)\n",
			__func__, ret);
		return ret;
	}

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT UREE_UnregisterSharedmem_ReadOnly(UREE_SESSION_HANDLE handle,
	UREE_SHAREDMEM_HANDLE shm_handle)
{
	return __UREE_UnregisterSharedmem(handle, shm_handle, 1);
}

#if 1
/*For Secure Carema Application Debug*/
struct user_test_chm_param {
	uint32_t append_chm_session;
	uint32_t alloc_chm_session;
	uint32_t append_chm_handle;
};

struct user_sc_param {
	uint32_t size;         /*alloc chm size*/
	uint32_t alignment;    /*alloc alignment size*/
	uint32_t ION_handle;
	uint32_t other_handle; /*shm handle*/
	struct user_test_chm_param chmp;
};

TZ_RESULT UREE_ION_CP_Chm2Shm(UREE_SESSION_HANDLE session,
	UREE_SHAREDMEM_HANDLE shm_handle, UREE_ION_HANDLE ION_Handle,
	uint32_t size)
{
	struct user_sc_param p;
	int iret;

	ALOGI("UREE_ION_CP_Chm2Shm\n");

	if (gz_fd < 0) {
		ALOGE("%s invalid fd %d\n", __FUNCTION__, gz_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	p.chmp.alloc_chm_session = session;
	p.ION_handle = ION_Handle;
	p.other_handle = shm_handle; /*shm handle*/
	p.size = size;

	iret = ioctl(gz_fd, MTEE_CMD_SC_TEST_CP_CHM2SHM, &p);

	if (iret != TZ_RESULT_SUCCESS) {
		ALOGE("%s: ioctl ret %d, err %d (%s))\n", __func__, iret,
			errno, strerror(errno) );
		return iret;
	}

	return TZ_RESULT_SUCCESS;
}

/*only for Test: update chmem data*/
TZ_RESULT UREE_ION_TEST_UPT_CHMDATA(UREE_ION_HANDLE ION_Handle, uint32_t size)
{
	struct user_sc_param p;
	int iret;

	ALOGI("UREE_ION_TEST_UPT_CHMDATA\n");

	if (gz_fd < 0) {
		ALOGE("%s invalid fd %d\n", __FUNCTION__, gz_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	p.ION_handle = ION_Handle;
	p.size = size;

	iret = ioctl(gz_fd, MTEE_CMD_SC_TEST_UPT_CHMDATA, &p);

	if (iret != TZ_RESULT_SUCCESS) {
		ALOGE("%s: ioctl ret %d, err %d (%s))\n", __func__, iret,
			errno, strerror(errno) );
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT UREE_ION_TO_SHM_HANDLE(UREE_ION_HANDLE ION_Handle,
	UREE_SHAREDMEM_HANDLE *shm_handle)
{
	struct user_sc_param p;
	int iret;

	ALOGI("UREE_ION_TO_SHM_HANDLE\n");

	if (gz_fd < 0) {
		ALOGE("%s invalid fd %d\n", __FUNCTION__, gz_fd);
		return TZ_RESULT_ERROR_GENERIC;
	}

	p.ION_handle = ION_Handle;

	iret = ioctl(gz_fd, MTEE_CMD_SC_CHMEM_HANDLE, &p);
	if (iret != TZ_RESULT_SUCCESS) {
		ALOGE("%s: ioctl ret 0x%x, err %d (%s))\n", __func__, iret,
			errno, strerror(errno) );
		return TZ_RESULT_ERROR_GENERIC;
	}

	*shm_handle = p.other_handle;

	return TZ_RESULT_SUCCESS;
}

#endif

TZ_RESULT UREE_FOD_ShareMem_Enable(int on)
{
	TZ_RESULT ret;

	if (gz_fd < 0) {
		ret = UREE_OpenGzFd();
		if (ret != TZ_RESULT_SUCCESS)
			return ret;
	}

	if (on)
		ret = ioctl(gz_fd, MTEE_CMD_FOD_TEE_SHM_ON, NULL);
	else
		ret = ioctl(gz_fd, MTEE_CMD_FOD_TEE_SHM_OFF, NULL);

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT UREE_DeepIdle_Mask(bool flag)
{
	TZ_RESULT ret;

	if (gz_fd < 0) {
		ret = UREE_OpenGzFd();
		if (ret != TZ_RESULT_SUCCESS)
			return ret;
	}

	if (flag == true)
		ret = ioctl(gz_fd, MTEE_CMD_DEEP_IDLE_MASK, NULL);
	else
		ret = ioctl(gz_fd, MTEE_CMD_DEEP_IDLE_UNMASK, NULL);

	return ret;
}

TZ_RESULT UREE_SecureCamera_to_MTEE(UREE_SESSION_HANDLE session,
	SecureCamera_IMG_OBJ *sc_obj, int cmd)
{
	struct kree_scamera_param sp;
	MTEEC_PARAM p[4];

	int iret;

	sp.format = sc_obj->format;
	sp.height = sc_obj->height;
	sp.size   = sc_obj->size;
	sp.stride = sc_obj->stride;
	sp.width  = sc_obj->width;
	sp.cmd    = cmd;

	printf("[%s][%d] calling:sp. size=0x%x, width=%d, height=%d, stride=0x%x, format=%d\n",
		__func__, __LINE__, sp.size, sp.width, sp.height, sp.stride,
		sp.format);

	p[0].value.a = (int) sp.format;
	p[0].value.b = (int) sp.height;
	p[1].value.a = (uint32_t) sp.size;
	p[1].value.b = (uint32_t) sp.stride;
	p[2].value.a = (int) sp.width;
	p[2].value.b = (int) 0;
	p[3].value.a = (int) sp.cmd;
	p[3].value.b = (int) 0;

	iret = UREE_TeeServiceCall(session, TZCMD_CONNECT_CA_to_HA,
		TZ_ParamTypes4(TZPT_VALUE_INOUT, TZPT_VALUE_INOUT, TZPT_VALUE_INOUT,
		TZPT_VALUE_INOUT), p);
	if (iret!= TZ_RESULT_SUCCESS) {
		ALOGE ("[%s] ====> echo_server [0x9000] returns Fail. ret=0x%x\n",
			__func__, iret);
		return iret;
	}

	sp.done = p[3].value.b;
	if (sp.done == 1)
		return TZ_RESULT_SUCCESS;
	else
		return TZ_RESULT_ERROR_GENERIC;
}

