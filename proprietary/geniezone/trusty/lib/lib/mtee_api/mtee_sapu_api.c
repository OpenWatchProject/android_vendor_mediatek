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

#include <lib/mtee/mtee_sapu_happ.h>
#include <tz_private/ta_sys_mem.h>
#include <tz_private/log.h>
//#include <tz_cross/ta_test.h>

//#define MTEE_MEM_API_LOG(fmt...) printf("[MTEE MEM API] "fmt)
#define LOG_TAG "SAPU_API"


TZ_RESULT ioctl_sapu(const char *tag, MTEE_SAPU_CMD cmd, void *ptr)
{
	int rc;

	rc = ioctl(SYS_MTEE_SAPU_FD, cmd, ptr);
	if (rc != NO_ERROR) {
		ERR_LOG("ioctl %s failed. rc=%d\n", tag, rc);
		return TZ_RESULT_ERROR_GENERIC;
	}
	return TZ_RESULT_SUCCESS;
}


void *SAPU_AcquireSapuMteeShm(uint32_t *size)
{
	int rc;
	sapu_mtee_shm_t sapu_mtee_shm;
	sapu_mtee_shm.data = 0;
	sapu_mtee_shm.size = 0;
	rc = ioctl_sapu("QuerySapuMteeShm", MTEE_SAPU_ACQUIRE_SAPU_MTEE_SHM, &sapu_mtee_shm);
	DBG_LOG("%s va(0x%x) size(0x%x) rc(%d)\n", __func__, sapu_mtee_shm.data, sapu_mtee_shm.size, rc);
	if (rc) {
		return NULL;
	}
	*size = sapu_mtee_shm.size;
	return (void *)sapu_mtee_shm.data;
}

int SAPU_ReleaseSapuMteeShm(void **ptr, uint32_t *size)
{
	int rc;
	sapu_mtee_shm_t sapu_mtee_shm;
	if (*ptr==0 || *size==0) {
		ERR_LOG("%s ptr(%p), *ptr=0x%x\n", __func__, ptr, *((uint32_t *)ptr));
		return -1;
	}

	sapu_mtee_shm.data = *((uint32_t *)ptr);
	sapu_mtee_shm.size = *size;
	rc = ioctl_sapu("ReleaseSapuMteeShm", MTEE_SAPU_RELEASE_SAPU_MTEE_SHM, &sapu_mtee_shm);
	if (!rc) {
		*ptr = 0;
		*size = 0;
	}
	return rc;
}

int SAPU_SDSP_Run(	u32 sdsp1_buf_offset, u32 sdsp1_buf_num, u32 sdsp1_prop_offset, u32 sdsp1_prop_size,
					u32 sdsp2_buf_offset, u32 sdsp2_buf_num, u32 sdsp2_prop_offset, u32 sdsp2_prop_size,
					u32 reserved, bool sw_op_enabled, sapu_reg_info_t *reg_info)
{
	int rc = 0;
	sapu_cmd_area_info_t sapu_cmd_area_info;
	sapu_cmd_area_info.sdsp1_buf_num = sdsp1_buf_num;
	sapu_cmd_area_info.sdsp1_prop_size = sdsp1_prop_size;
	sapu_cmd_area_info.sdsp2_buf_num = sdsp2_buf_num;
	sapu_cmd_area_info.sdsp2_prop_size = sdsp2_prop_size;
	if (reg_info != NULL) {
		sapu_cmd_area_info.sdsp1_handle = reg_info->sdsp1_handle;
		sapu_cmd_area_info.sdsp1_reg_cmd = reg_info->sdsp1_cmd;
		sapu_cmd_area_info.sdsp1_reg_result = reg_info->sdsp1_result;
		sapu_cmd_area_info.sdsp2_handle = reg_info->sdsp2_handle;
		sapu_cmd_area_info.sdsp2_reg_cmd = reg_info->sdsp2_cmd;
		sapu_cmd_area_info.sdsp2_reg_result = reg_info->sdsp2_result;
	} else {
		sapu_cmd_area_info.sdsp1_reg_cmd = 0;
		sapu_cmd_area_info.sdsp2_reg_cmd = 0;
		sapu_cmd_area_info.sdsp1_reg_result = 0;
		sapu_cmd_area_info.sdsp2_reg_result = 0;
	}
	sapu_cmd_area_info.reserved = reserved;
	sapu_cmd_area_info.sw_op_enabled = sw_op_enabled;

	rc = ioctl_sapu("SDSP_Run", MTEE_SAPU_SDSP_RUN, &sapu_cmd_area_info);

	if (!rc && reg_info!=NULL) {
		reg_info->sdsp1_result = sapu_cmd_area_info.sdsp1_reg_result;
		reg_info->sdsp2_result = sapu_cmd_area_info.sdsp2_reg_result;
	}

	return rc;
}

int SAPU_v2_SDSP_Run(	u32 sdsp1_buf_num, u32 sdsp1_prop_size, u32 sdsp2_buf_num, u32 sdsp2_prop_size,
						u32 sdsp3_buf_num, u32 sdsp3_prop_size, u32 sdsp4_buf_num, u32 sdsp4_prop_size,
						u32 reserved, bool sw_op_enabled, sapu_reg_v2_info_t *reg_v2_info)
{
	int rc = 0;
	sapu_cmd_area_info_t sapu_cmd_area_info;
	sapu_cmd_area_info.sdsp1_buf_num = sdsp1_buf_num;
	sapu_cmd_area_info.sdsp1_prop_size = sdsp1_prop_size;
	sapu_cmd_area_info.sdsp2_buf_num = sdsp2_buf_num;
	sapu_cmd_area_info.sdsp2_prop_size = sdsp2_prop_size;
	sapu_cmd_area_info.sdsp3_buf_num = sdsp3_buf_num;
	sapu_cmd_area_info.sdsp3_prop_size = sdsp3_prop_size;
	sapu_cmd_area_info.sdsp4_buf_num = sdsp4_buf_num;
	sapu_cmd_area_info.sdsp4_prop_size = sdsp4_prop_size;
	if (reg_v2_info != NULL) {
		sapu_cmd_area_info.sdsp1_handle = reg_v2_info->sdsp1_handle;
		sapu_cmd_area_info.sdsp1_reg_cmd = reg_v2_info->sdsp1_cmd;
		sapu_cmd_area_info.sdsp1_reg_result = reg_v2_info->sdsp1_result;
		sapu_cmd_area_info.sdsp2_handle = reg_v2_info->sdsp2_handle;
		sapu_cmd_area_info.sdsp2_reg_cmd = reg_v2_info->sdsp2_cmd;
		sapu_cmd_area_info.sdsp2_reg_result = reg_v2_info->sdsp2_result;
		sapu_cmd_area_info.sdsp3_handle = reg_v2_info->sdsp3_handle;
		sapu_cmd_area_info.sdsp3_reg_cmd = reg_v2_info->sdsp3_cmd;
		sapu_cmd_area_info.sdsp3_reg_result = reg_v2_info->sdsp3_result;
		sapu_cmd_area_info.sdsp4_handle = reg_v2_info->sdsp4_handle;
		sapu_cmd_area_info.sdsp4_reg_cmd = reg_v2_info->sdsp4_cmd;
		sapu_cmd_area_info.sdsp4_reg_result = reg_v2_info->sdsp4_result;
	} else {
		sapu_cmd_area_info.sdsp1_reg_cmd = 0;
		sapu_cmd_area_info.sdsp2_reg_cmd = 0;
		sapu_cmd_area_info.sdsp3_reg_cmd = 0;
		sapu_cmd_area_info.sdsp4_reg_cmd = 0;
		sapu_cmd_area_info.sdsp1_reg_result = 0;
		sapu_cmd_area_info.sdsp2_reg_result = 0;
		sapu_cmd_area_info.sdsp3_reg_result = 0;
		sapu_cmd_area_info.sdsp4_reg_result = 0;
	}
	sapu_cmd_area_info.reserved = reserved;
	sapu_cmd_area_info.sw_op_enabled = sw_op_enabled;

	rc = ioctl_sapu("SDSP_Run", MTEE_SAPU_SDSP_RUN, &sapu_cmd_area_info);

	if (!rc && reg_v2_info!=NULL) {
		reg_v2_info->sdsp1_result = sapu_cmd_area_info.sdsp1_reg_result;
		reg_v2_info->sdsp2_result = sapu_cmd_area_info.sdsp2_reg_result;
		reg_v2_info->sdsp3_result = sapu_cmd_area_info.sdsp3_reg_result;
		reg_v2_info->sdsp4_result = sapu_cmd_area_info.sdsp4_reg_result;
	}

	return rc;
}

int SAPU_SDSP_Core1_Run(u32 sdsp1_buf_offset, u32 sdsp1_buf_num, u32 sdsp1_prop_offset, u32 sdsp1_prop_size,
						u32 reserved, sapu_reg_info_t *reg_info)
{
	int rc = 0;
	sapu_cmd_area_info_t sapu_cmd_area_info;
	sapu_cmd_area_info.sdsp1_buf_num = sdsp1_buf_num;
	sapu_cmd_area_info.sdsp1_prop_size = sdsp1_prop_size;
	if (reg_info != NULL) {
		sapu_cmd_area_info.sdsp1_handle = reg_info->sdsp1_handle;
		sapu_cmd_area_info.sdsp1_reg_cmd = reg_info->sdsp1_cmd;
		sapu_cmd_area_info.sdsp1_reg_result = reg_info->sdsp1_result;
	} else {
		sapu_cmd_area_info.sdsp1_reg_cmd = 0;
		sapu_cmd_area_info.sdsp1_reg_result = 0;
	}
	sapu_cmd_area_info.reserved = reserved;

	rc = ioctl_sapu("SDSP1_Run", MTEE_SAPU_SDSP_CORE1_RUN, &sapu_cmd_area_info);

	if (!rc && reg_info!=NULL) {
		reg_info->sdsp1_result = sapu_cmd_area_info.sdsp1_reg_result;
	}

	return rc;
}

int SAPU_SDSP_Core2_Run(u32 sdsp2_buf_offset, u32 sdsp2_buf_num, u32 sdsp2_prop_offset, u32 sdsp2_prop_size,
						u32 reserved, sapu_reg_info_t *reg_info)
{
	int rc = 0;
	sapu_cmd_area_info_t sapu_cmd_area_info;
	sapu_cmd_area_info.sdsp2_buf_num = sdsp2_buf_num;
	sapu_cmd_area_info.sdsp2_prop_size = sdsp2_prop_size;
	if (reg_info != NULL) {
		sapu_cmd_area_info.sdsp2_handle = reg_info->sdsp2_handle;
		sapu_cmd_area_info.sdsp2_reg_cmd = reg_info->sdsp2_cmd;
		sapu_cmd_area_info.sdsp2_reg_result = reg_info->sdsp2_result;
	} else {
		sapu_cmd_area_info.sdsp2_reg_cmd = 0;
		sapu_cmd_area_info.sdsp2_reg_result = 0;
	}
	sapu_cmd_area_info.reserved = reserved;

	rc = ioctl_sapu("SDSP2_Run", MTEE_SAPU_SDSP_CORE2_RUN, &sapu_cmd_area_info);

	if (!rc && reg_info!=NULL) {
		reg_info->sdsp2_result = sapu_cmd_area_info.sdsp2_reg_result;
	}

	return rc;
}

int SAPU_Log_Dump(u32 mask)
{
	int rc = 0;
	rc = ioctl_sapu("Log_Dump", MTEE_SAPU_LOG_DUMP, &mask);
	return rc;
}

int SAPU_SDSP_BOOTUP(void)
{
	int rc = 0;
	rc = ioctl_sapu("SDSP_BOOT", MTEE_SAPU_BOOTUP, NULL);
	return rc;
}

int SAPU_GetTeeMemInfo(uint32_t *VA1, uint32_t *VA2, u32 sdsp1_buf_offset, u32 sdsp1_buf_num, u32 sdsp1_prop_offset, u32 sdsp1_prop_size, uint64_t *basePA)
{
	int rc = 0;
	#if 0
	sapu_cmd_area_info_t sapu_cmd_area_info;
	sapu_cmd_area_info.sdsp1_buf_offset = sdsp1_buf_offset;
	sapu_cmd_area_info.sdsp1_buf_num = sdsp1_buf_num;
	sapu_cmd_area_info.sdsp1_prop_offset = sdsp1_prop_offset;
	sapu_cmd_area_info.sdsp1_prop_size = sdsp1_prop_size;

	rc = ioctl_sapu("SDSP_GetVA", MTEE_SAPU_GET_TEE_MEM_INFO, &sapu_cmd_area_info);

	*VA1 = sapu_cmd_area_info.sdsp1_buf_offset;//fix me, temporarily...
	*VA2 = sapu_cmd_area_info.sdsp1_prop_offset;//fix me, temporarily...
	*basePA = sapu_cmd_area_info.basePA;
	#endif

	return rc;
}

void *SAPU_AcquireSapuTeeShm(uint32_t *size)
{
	int rc;
	sapu_tee_shm_t sapu_tee_shm;
	sapu_tee_shm.data = 0;
	sapu_tee_shm.size = 0;
	rc = ioctl_sapu("QuerySapuTeeShm", MTEE_SAPU_ACQUIRE_SAPU_TEE_SHM, &sapu_tee_shm);
	DBG_LOG("%s va(0x%x) size(0x%x) rc(%d)\n", __func__, sapu_tee_shm.data, sapu_tee_shm.size, rc);
	if (rc) {
		return NULL;
	}
	*size = sapu_tee_shm.size;
	return (void *)sapu_tee_shm.data;
}

int SAPU_ReleaseSapuTeeShm(void **ptr, uint32_t *size)
{
	int rc;
	sapu_tee_shm_t sapu_tee_shm;
	if (*ptr==0 || *size==0) {
		ERR_LOG("%s ptr(%p), *ptr=0x%x\n", __func__, ptr, *((uint32_t *)ptr));
		return -1;
	}

	sapu_tee_shm.data = *((uint32_t *)ptr);
	sapu_tee_shm.size = *size;
	rc = ioctl_sapu("ReleaseSapuTeeShm", MTEE_SAPU_RELEASE_SAPU_TEE_SHM, &sapu_tee_shm);
	if (!rc) {
		*ptr = 0;
		*size = 0;
	}
	return rc;
}

uint64_t SAPU_vDSP_Shm_offset(uint64_t ptr)
{
	int rc;

	rc = ioctl_sapu("Get ptr offset", MTEE_SAPU_vDSP_SHM_OFFSET, &ptr);
	if (rc) {
		return NULL;
	}

	return ptr;
}

