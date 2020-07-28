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
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <trusty_std.h>
#include <mtee_server.h>
#include <tz_private/ta_sys_mem.h>
#include <tz_private/sys_ipc.h>
#include <tz_private/log.h>
#include <tz_cross/ta_test.h>

#include <lib/mtee/mtee_sys.h>
#include <lib/mtee/mtee_location.h>

#define LOG_TAG "SharedMemOppo"

#define ALIGHMENT_SIZE 1024
#define IMG_SIZE (128*ALIGHMENT_SIZE)
#define COMPARE_LOOP (IMG_SIZE/4)
MTEE_SECUREMEM_HANDLE TA_TAs_sec_handle = 0;
uint8_t *TA_TAs_sec_va;
uint32_t TA_TAs_sec_size;

void test_shared_mem_HAs_alloc(MTEEC_PARAM *param)
{
	TZ_RESULT tz_ret;
	MTEE_MEM_PARAM TAs_sec_info;

	param[3].value.a = 0xcc;

	// Create buf between TAs, just get handle
	TA_Mem_AllocMem(&TA_TAs_sec_handle, ALIGHMENT_SIZE, IMG_SIZE);

	// Query (buf between TAs) va
	tz_ret = TA_Mem_QueryMem(TA_TAs_sec_handle, &TAs_sec_info);
	if(tz_ret == TZ_RESULT_SUCCESS) {
		if (TAs_sec_info.buffer != NULL) {
			TA_TAs_sec_va = (uint8_t *)TAs_sec_info.buffer;
			TA_TAs_sec_size = (uint32_t)TAs_sec_info.size;
			if (TA_TAs_sec_size != IMG_SIZE) {
				DBG_LOG("image size(%u) != IMG_SIZE\n", TA_TAs_sec_size);
				return;
			}
		}
	}

	DBG_LOG("mem write handle(0x%x) ptr(%p) size(%u) %s\n", TA_TAs_sec_handle, TA_TAs_sec_va, TA_TAs_sec_size, __func__);
	param[2].value.a = TA_TAs_sec_handle;
	param[3].value.a = 0x66;
}

void test_shared_mem_ha_ta_setup(MTEEC_PARAM *param)
{
	TZ_RESULT tz_ret;
	uint32_t fill_data = param[0].value.a;
	uint32_t HA_HATA_ion_shm_handle = param[0].value.b;
	MTEE_MEM_PARAM HA_TA_secshm_info;
	param[3].value.a = 0xcc;
	uint32_t *HA_TA_secshm_va;
	uint32_t HA_TA_secshm_size;
	uint32_t i;
	DBG_LOG("HA_HATA_ion_shm_handle(0x%x) %s\n", HA_HATA_ion_shm_handle, __func__);

	tz_ret = TA_Mem_QueryChunkmem(HA_HATA_ion_shm_handle, &HA_TA_secshm_info, 0);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		DBG_LOG("TA_Mem_QueryChunkmem Fail. tz_ret=0x%x\n", tz_ret);
		return;
	}
	HA_TA_secshm_va = (uint32_t *)HA_TA_secshm_info.buffer;
	HA_TA_secshm_size = (uint32_t)HA_TA_secshm_info.size;
	DBG_LOG("HA_TA buf(%p) size(%u) %s\n", HA_TA_secshm_va, HA_TA_secshm_size, __func__);

	for (i=0 ; i<(HA_TA_secshm_size/4) ; i++) {
		HA_TA_secshm_va[i] = fill_data;
	}
	param[3].value.a = 0x66;
}

void test_shared_mem_HAs_copy(MTEEC_PARAM *param)
{
	TZ_RESULT tz_ret;
	void *CA_HA1_shm_va;
	MTEE_SHAREDMEM_HANDLE HA1_CAHA1_shm_handle;
	MTEE_SHAREDMEM_PARAM CA_HA1_shm_info;	/*shm*/

	uint32_t HA_HATA_ion_shm_handle = param[0].value.a;
	MTEE_MEM_PARAM HA_TA_secshm_info;
	uint32_t *HA_TA_secshm_va;
	uint32_t HA_TA_secshm_size;

	param[3].value.a = 0xcc;
	int ret;

	tz_ret = TA_Mem_QueryChunkmem(HA_HATA_ion_shm_handle, &HA_TA_secshm_info, 0);
	if (tz_ret != TZ_RESULT_SUCCESS) {
		DBG_LOG("TA_Mem_QueryChunkmem Fail. tz_ret=0x%x\n", tz_ret);
		return;
	}
	HA_TA_secshm_va = (uint32_t *)HA_TA_secshm_info.buffer;
	HA_TA_secshm_size = (uint32_t)HA_TA_secshm_info.size;
	DBG_LOG("HA_TA buf(%p) size(%u) %s\n", HA_TA_secshm_va, HA_TA_secshm_size, __func__);
	memcpy(TA_TAs_sec_va, HA_TA_secshm_va, TA_TAs_sec_va<HA_TA_secshm_size?TA_TAs_sec_va:HA_TA_secshm_size);

	if ( param[0].value.b !=0 )
	{
		HA1_CAHA1_shm_handle = param[0].value.b;
		ret = TA_Mem_QuerySharedmem(HA1_CAHA1_shm_handle, &CA_HA1_shm_info);
		if (ret) {
			ERR_LOG("TA_Mem_QuerySharedmem error.[ret=%d], return %s\n", ret, __func__);
			return;
		} else {
			CA_HA1_shm_va = (void *) CA_HA1_shm_info.buffer;
			DBG_LOG("query share mem(%p) size(%d) %s\n", CA_HA1_shm_va, CA_HA1_shm_info.size, __func__);
		}
		memcpy(CA_HA1_shm_va, TA_TAs_sec_va, CA_HA1_shm_info.size);
	}

	param[3].value.a = 0x66;
}

void test_shared_mem_HAs_free(MTEEC_PARAM *param)
{
	int ret;
	param[3].value.a = 0xcc;
	ret = TA_Mem_UnreferenceMem(TA_TAs_sec_handle);
	if (ret==0) param[3].value.a = 0x66;
	DBG_LOG("ret =%d %s\n", ret, __func__);
}