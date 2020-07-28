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
#include <tz_private/log.h>
#include <tz_private/system.h>


#define LOG_TAG "MTEE DRV API"

TZ_RESULT MTEE_DrvSetUuid(const char *ta_uuid, unsigned int drv_id)
{
	int rc;
	user_drv_t param;

	if(ta_uuid == NULL || drv_id == 0)
	{
		ERR_LOG("[ERROR] ta_uuid cannot be null and drv_id cannot be 0 \n");
		return TZ_RESULT_ERROR_BAD_PARAMETERS;
	}

	strncpy(param.uuid, ta_uuid, MTEE_DRV_UUID_LEN);
	param.drv_id = drv_id;

	rc = ioctl(SYS_USER_MTEE_FD, MTEE_DRV_SET_UUID, &param);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl MTEE_DRV_SET_UUID failed. rc = %d\n", rc);
		return TZ_RESULT_ERROR_GENERIC;
	}

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_DrvGetUuid(char *ta_uuid, unsigned int drv_id)
{
	int rc;
	user_drv_t param;

	param.drv_id = drv_id;
	rc = ioctl(SYS_USER_MTEE_FD, MTEE_DRV_GET_UUID, &param);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl MTEE_DRV_SET_UUID failed. rc = %d\n", rc);
		return TZ_RESULT_ERROR_GENERIC;
	}

	strncpy(ta_uuid, param.uuid, MTEE_DRV_UUID_LEN);

	return TZ_RESULT_SUCCESS;
}


