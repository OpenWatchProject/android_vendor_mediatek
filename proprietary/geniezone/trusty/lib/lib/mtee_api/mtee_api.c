/*
 * Copyright (C) 2012-2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <trusty_std.h>

#include <lib/mtee/mtee_srv.h>
#include <tz_private/system.h>
#include <tz_private/log.h>

//#define MTEE_API_LOG(fmt...) printf("[MTEE API]"fmt)
#define LOG_TAG "MTEE API"

TZ_RESULT ioctl_test(const char *tag, enum MTEE_USER_CMD cmd, void *ptr)
{
	int rc;

	rc = ioctl(SYS_USER_MTEE_FD, cmd, ptr);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl %s failed. rc = %d\n", tag, rc);
		return TZ_RESULT_ERROR_GENERIC;
	}
	return TZ_RESULT_SUCCESS;
}


/**************** User Space Handle APIs *******************/
MTEE_HANDLE_TYPE_ID MTEE_HandleInit(const char *hname)
{
	user_handle_t h_param;

	strncpy(h_param.type, hname, MTEE_HANDLE_TYPE_LEN);
	//DBG_LOG("HANDLE_NEW_TYPE: type name = %s\n", h_param.type);

	if ( ioctl_test("HANDLE_NEW_TYPE", MTEE_HANDLE_NEW_TYPE, &h_param) == TZ_RESULT_SUCCESS )
		return h_param.handle;
	else
		return 0;
}

TZ_RESULT MTEE_HandleAllocate(MTEE_HANDLE_TYPE_ID hid, void *object, uint32_t *pHandle)
{
	if (hid == 0)
    {
    	ERR_LOG("HandleAllocate: Invalid handle type id. \n");
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }

	user_handle_t h_param;

	h_param.type_p  = (uint32_t)hid;
	h_param.obj = (uint32_t)object;
	//DBG_LOG("HANDLE_NEW: hid = 0x%x, obj = 0x%x,\n", h_param.type_p, h_param.obj);

	if ( ioctl_test("HANDLE_NEW", MTEE_HANDLE_NEW, &h_param) == TZ_RESULT_SUCCESS ) {
		*pHandle = h_param.handle;
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}

void* MTEE_HandleGetObject(MTEE_HANDLE_TYPE_ID hid, uint32_t handle)
{
	if (hid == 0)
    {
    	ERR_LOG("Invalid handle type id. \n");
        return NULL;
    }
	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return NULL;
    }

	user_handle_t h_param;
	h_param.type_p  = (uint32_t)hid;
	h_param.handle = handle;

	//DBG_LOG("HANDLE_GET_OBJ: hid = 0x%x, handle = 0x%x,\n", h_param.type_p, h_param.handle);

	if ( ioctl_test("HANDLE_GET_OBJ", MTEE_HANDLE_GET_OBJ, &h_param) == TZ_RESULT_SUCCESS ) {
		return (void*)h_param.obj;
	} else
		return NULL;
}

void MTEE_HandlePutObject(MTEE_HANDLE_TYPE_ID hid, uint32_t handle)
{
	TZ_RESULT tz_ret;

	if (hid == 0)
    {
    	ERR_LOG("Invalid handle type id. \n");
        return ;
    }
	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return ;
    }

	user_handle_t h_param;
	h_param.type_p  = (uint32_t)hid;
	h_param.handle = handle;

	//DBG_LOG("HANDLE_PUT_OBJ: hid = 0x%x, handle = 0x%x,\n", h_param.type_p, h_param.handle);

	tz_ret = ioctl_test("HANDLE_PUT_OBJ", MTEE_HANDLE_PUT_OBJ, &h_param);
	if ( tz_ret != TZ_RESULT_SUCCESS ) {
		ERR_LOG("HANDLE_PUT_OBJ returned with error \n");
	}
}

TZ_RESULT MTEE_HandleFree(MTEE_HANDLE_TYPE_ID hid, uint32_t handle)
{
	user_handle_t h_param;

	if (hid == 0)
    {
    	ERR_LOG("Invalid handle type id. \n");
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }
	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }

	h_param.type_p  = (uint32_t)hid;
	h_param.handle = handle;

	//DBG_LOG("HANDLE_FREE: hid = 0x%x, handle = 0x%x,\n", h_param.type_p, h_param.handle);

	if ( ioctl_test("HANDLE_FREE", MTEE_HANDLE_FREE, &h_param) == TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}

/**************** User Space Session APIs *******************/
TZ_RESULT MTEE_CreateSession(const char *ta_uuid, MTEE_SESSION_HANDLE *pHandle)
{
	user_session_t s_param;

	strncpy(s_param.name, ta_uuid, MTEE_SESSION_NAME_LEN);

	if ( ioctl_test("SESSION_CREATE", MTEE_SESSION_CREATE, &s_param) == TZ_RESULT_SUCCESS ) {
		*pHandle = s_param.handle;
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}

TZ_RESULT MTEE_CopyRunCallbacks(MTEE_SESSION_HANDLE handle, uint32_t num)
{
	int i;
	cb_node_t *ptr;
    session_cbs_t *param;
	MTEE_ResourceFreeFunction cb;

	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }

	// prepare buffer
	//cbs = (cb_node_t*)malloc(num * sizeof(cb_node_t));
	param = (session_cbs_t *)malloc( sizeof(session_cbs_t) + num * sizeof(cb_node_t));
    param->handle = handle;
    param->cb_num = num;

	// call ioctl to copy callbacks back
	if ( ioctl_test("SESSION_GET_CBS", MTEE_SESSION_GET_CBS, param) != TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_ERROR_GENERIC;
	}

	// for all callback, run & unregister
	ptr = &(param->buffer);
	for (i = 0; i < num; i++) {
		cb = (MTEE_ResourceFreeFunction)(ptr->callback);
		cb(ptr->data);
		ptr++;
	}

	free(param);
	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_CloseSession(MTEE_SESSION_HANDLE handle)
{
	TZ_RESULT ret;
	user_session_t s_param;
	session_cbs_t c_param;

	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }

	s_param.handle = handle;
	c_param.handle = handle;
	//DBG_LOG("SESSION_CLOSE: handle = 0x%x\n", handle);

	// Query the number of registered resource
	if ( ioctl_test("SESSION_GET_CB_NUM", MTEE_SESSION_GET_CB_NUM, &c_param) != TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_ERROR_GENERIC;
	}
	//DBG_LOG("SESSION_CLOSE: number of cb = %u\n", c_param.cb_num);

	// If any, copy callbacks back and call in userspace
	if (c_param.cb_num > 0) {
		ret = MTEE_CopyRunCallbacks(handle, c_param.cb_num);
		if (ret != TZ_RESULT_SUCCESS) ERR_LOG("there are callbacks but failed to copy back\n");
	}

	if ( ioctl_test("SESSION_CLOSE", MTEE_SESSION_CLOSE, &s_param) == TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}

TZ_RESULT MTEE_SetSessionUserData(MTEE_SESSION_HANDLE handle, void *pUserData)
{
	user_session_t s_param;

	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }

	DBG_LOG("SESSION_SET_DATA: handle = 0x%x, data=0x%x\n", handle, pUserData);
	s_param.handle = handle;
	s_param.res = (uint64_t)pUserData;

	if ( ioctl_test("SESSION_PUT_OBJ", MTEE_SESSION_PUT_OBJ, &s_param) == TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}

TZ_RESULT MTEE_GetSessionUserData(MTEE_SESSION_HANDLE handle, void **ppUserData)
{
	user_session_t s_param;

	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }

	//DBG_LOG("SESSION_GET_DATA: handle = 0x%x\n", handle);
	s_param.handle = handle;

	if ( ioctl_test("SESSION_GET_OBJ", MTEE_SESSION_GET_OBJ, &s_param) == TZ_RESULT_SUCCESS ) {
		*ppUserData = (void*)s_param.res;
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}

TZ_RESULT MTEE_RegisterSessionResource(MTEE_SESSION_HANDLE handle,
										MTEE_ResourceFreeFunction func,
										void *user_data)
{
	user_session_t s_param;

	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }

	//DBG_LOG("SESSION_REG_RSC: handle = 0x%x, data=0x%x\n", handle, user_data);
	s_param.handle = handle;
	s_param.callback = (uint64_t)func;
	s_param.res = (uint64_t)user_data;

	if ( ioctl_test("SESSION_REG_RSC", MTEE_SESSION_REG_RSC, &s_param) == TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;

}

TZ_RESULT MTEE_UnregisterSessionResource(MTEE_SESSION_HANDLE handle,
											MTEE_ResourceFreeFunction func,
											void *user_data)
{
	user_session_t s_param;

	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }

	//DBG_LOG("SESSION_UNREG_RSC: handle = 0x%x, data=0x%x\n", handle, user_data);
	s_param.handle = handle;
	s_param.callback = (uint64_t)func;
	s_param.res = (uint64_t)user_data;

	if ( ioctl_test("SESSION_UNREG_RSC", MTEE_SESSION_UNREG_RSC, &s_param) == TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}

int MTEE_QuerySessionResource(MTEE_SESSION_HANDLE handle,
								MTEE_ResourceFreeFunction func,
								void *user_data)
{
	user_session_t s_param;

	if (handle == 0)
    {
    	ERR_LOG("Invalid handle 0x%x. \n", handle);
        return TZ_RESULT_ERROR_INVALID_HANDLE;
    }

	//DBG_LOG("SESSION_QUERY_RSC: handle = 0x%x, data=0x%x\n", handle, user_data);
	s_param.handle = handle;
	s_param.callback = (uint64_t)func;
	s_param.res = (uint64_t)user_data;

	return ioctl(SYS_USER_MTEE_FD, MTEE_SESSION_QUERY_RSC, &s_param);
}

/**************** User Space HA dynamic loading APIs *******************/
TZ_RESULT MTEE_CreateHA(user_DL_t *info)
{
	DBG_LOG("info.chm_handle = %d\n",info->elf);

	if ( ioctl_test("HA_CREATE", MTEE_HA_CREATE, info) == TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}

TZ_RESULT MTEE_StartHA(user_DL_t info)
{
	if ( ioctl_test("HA_START", MTEE_HA_START, &info) == TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}

TZ_RESULT MTEE_UnloadHA(uuid_t app_uuid)
{
	if (ioctl_test("HA_UNLOAD", MTEE_HA_UNLOAD, &app_uuid) == TZ_RESULT_SUCCESS ) {
		return TZ_RESULT_SUCCESS;
	} else
		return TZ_RESULT_ERROR_GENERIC;
}


/************* User Space Utility Functions  **************/
void MTEE_USleep(int64_t ustime)
{
	int64_t t = 1000ULL * ustime;
	nanosleep(0, 0, t);
}

void MTEE_UDelay(unsigned long ustime)
{
	int64_t start, now, diff = 0;
	int64_t nstime = ustime * 1000;

	gettime(0, 0, &start);
	while(diff < nstime) {
        gettime(0, 0, &now);
		diff = now - start;
	}
}

TZ_RESULT MTEE_DumpAllThreads(void)
{
	int rc;
	user_thread_t param;

	rc = ioctl(SYS_USER_MTEE_FD, MTEE_THREAD_DUMP_ALL, &param);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl MTEE_THREAD_DUMP_ALL failed. rc = %d\n", rc);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_CurrentThread(uint64_t *threadid)
{
	int rc;
	uint64_t id;

	rc = ioctl(SYS_USER_MTEE_FD, MTEE_THREAD_CURRENT_THREAD, &id);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl MTEE_THREAD_CURRENT_THREAD failed. rc = %d\n", rc);
		return TZ_RESULT_ERROR_GENERIC;
	}

	*threadid =  id;
	//DBG_LOG("[%s]  threadid = 0x%llx\n", __func__, *threadid);

	return TZ_RESULT_SUCCESS;
}

/***************************************************************/
/*   sync with original MTEE   */
/***************************************************************/

TZ_RESULT MTEE_CreateSessionWithTag(const char *ta_uuid, MTEE_SESSION_HANDLE *pHandle, const char *tag)
{
#if 0
	return MTEE_CreateSession_Body(ta_uuid, pHandle, tag);
#endif

	print_mtee_orig_msg;
	return; //not support
}

/*fix mtee sync*/
TZ_RESULT MTEE_HandleAllocateWithTag(MTEE_HANDLE_TYPE_ID hid,
                             void *object, uint32_t *pHandle,
                             const char *tag)
{
#if 0
    return MTEE_HandleAllocate_Body(hid, object, pHandle, tag);
#endif

	print_mtee_orig_msg;
	return; //not support
}

void MTEE_Puts(const char *buf)
{
#if 0
    char *ptr = MTEE_GetReeParamAddress();
    int num, len = strlen(buf);

    while (len)
    {
        num = (len < REE_SERVICE_BUFFER_SIZE) ? len : (REE_SERVICE_BUFFER_SIZE-1);
        strncpy(ptr, buf, num);
        len -= num;
        buf += num;
        ptr[num] = 0;
        MTEE_ReeServiceCall(REE_SERV_PUTS);
    }
#endif

	print_mtee_orig_msg;
	return; //not support
}

