/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <msee_fwk/drv_call.h>
#include <tz_private/system.h>
#include <tz_private/log.h>
#include <tz_cross/ta_drv.h>

#define LOG_TAG "MSEE_DRV_CALL"
#define UUID_LEN 32
#define MEM_COPY_BUF_SIZE 512

int mdrv_open(unsigned int driver_id, void *param)
{
	TZ_RESULT ret;
	char uuid[UUID_LEN];
	MTEE_SESSION_HANDLE hSession;

	MTEEC_PARAM params[4];
	uint32_t paramTypes;

	ret = MTEE_DrvGetUuid(uuid, driver_id);
	if(ret != TZ_RESULT_SUCCESS)
	{
		ERR_LOG("[ERROR] no uuid for driver id %d \n", driver_id);
		return -1;
	}

	ret = UTEE_CreateSession(uuid, &hSession);
	if(ret != TZ_RESULT_SUCCESS)
	{
		ERR_LOG("[ERROR] create session fail for driver id %d \n", driver_id);
		return -1;
	}

	params[0].mem.buffer = param;
	params[0].mem.size = MEM_COPY_BUF_SIZE;
	paramTypes = TZ_ParamTypes1(TZPT_MEM_INOUT);

	ret = UTEE_TeeServiceCall(hSession, TZCMD_INT_DRV_OPEN, paramTypes, params);
	if(ret != TZ_RESULT_SUCCESS)
	{
		ERR_LOG("[ERROR] driver open callback function fail for driver id %d \n", driver_id);
		return -1;
	}

	return hSession;
}

int mdrv_ioctl(int handle, unsigned int cmd_id, void *param)
{
	TZ_RESULT ret;
	MTEE_SESSION_HANDLE hSession;

	MTEEC_PARAM params[4];
	uint32_t paramTypes;

	hSession = handle;
	params[0].value.a = cmd_id;
	params[1].mem.buffer = param;
	params[1].mem.size = MEM_COPY_BUF_SIZE;
	paramTypes = TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_MEM_INOUT);

	ret = UTEE_TeeServiceCall(hSession, TZCMD_INT_DRV_IOCTL, paramTypes, params);
	if(ret != TZ_RESULT_SUCCESS)
	{
		ERR_LOG("[ERROR] driver ioctl callback function fail for driver handle %d \n", handle);
		return -1;
	}

	return 0;
}

int mdrv_close(int handle)
{
	TZ_RESULT ret;
	MTEE_SESSION_HANDLE hSession;

	MTEEC_PARAM params[4];
	uint32_t paramTypes;

	hSession = handle;
	paramTypes = TZ_ParamTypes(TZPT_NONE, TZPT_NONE, TZPT_NONE, TZPT_NONE);

	ret = UTEE_TeeServiceCall(hSession, TZCMD_INT_DRV_CLOSE, paramTypes, params);
	if(ret != TZ_RESULT_SUCCESS)
	{
		ERR_LOG("[ERROR] driver close callback function fail for driver handle %d \n", handle);
		return -1;
	}

	ret = UTEE_CloseSession(hSession);
	if(ret != TZ_RESULT_SUCCESS)
	{
		ERR_LOG("[ERROR] close session fail for driver handle %d \n", handle);
		return -1;
	}

	return 0;
}


