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

#include <tz_private/ta_sys_mem.h>
#include <tz_private/sys_mem.h>
#include <tz_private/log.h>
#include <tz_private/profile.h>
#include <tz_private/system.h>
#include <unittest.h>
#include <tz_cross/ta_test.h>
#include <tz_private/mtee_mmap_internal.h>
#include <unittest.h>
#include <lib/mtee/mtee_sys.h>
#include <trusty_syscalls.h>

#ifdef NEED_OPENMP
#include <libgomp.h>
#endif

#include <lib/storage/storage.h>
#include <mtee_server.h>
#include <tz_private/sys_ipc.h>
#include <lib/mtee/mtee_srv.h>

#define LOG_TAG "MULTICORE"

#include "gz-test_common.h"
#define TEST_LOOP (MALLOC_SIZE/1)

void test_omp_basic(void)
{
	int count;

	count = 0;

#ifdef NEED_OPENMP
	#pragma omp parallel
	{
		count++;
		printf("gr-libgomp-unittest #pragma omp parallel\n");
	}
	if (count!=omp_get_num_procs()) {
		printf("omp_core=%d, need %d\n", count, omp_get_num_procs());
	} else {
		printf("$$$$$ omp_core=%d $$$$$\n", count);
	}
#else
	printf("not support omp\n");
#endif
}

void test_omp_ut(uint8_t *source_a, uint8_t *source_b, float *source_one_core_sum, float *source_multi_core_sum, MTEEC_PARAM *param)
{
	int test_cnt=0;
	uint8_t *a, *b;
	float *one_core_sum, *multi_core_sum;

	if (source_a!=NULL && source_b!=NULL && source_one_core_sum!=NULL && source_multi_core_sum != NULL ) {
		a = source_a; b = source_b; one_core_sum = source_one_core_sum; multi_core_sum = source_multi_core_sum;
	} else {
		param[2].value.a=0xdeac;
		DBG_LOG("not get source_a fail\n");
		return;
	}

	DBG_LOG("init a, b data +\n");
	for (int i=0 ; i<TEST_LOOP ; i++) {
		a[i] = rand()%3U;
		b[i] = rand()%3U;
		one_core_sum[i] = 0;
		multi_core_sum[i] = 0;
		//if (i>100 && i<110) DBG_LOG("[%u %u]\n", (uint32_t)a[i], (uint32_t)b[i]);
	}
	DBG_LOG("init a, b data -\n");

	#define TEST_MAX 3
	for (test_cnt=1 ; test_cnt<=TEST_MAX ; test_cnt++)
	{
		DBG_LOG("start 1_core computing +\n");
		for (int i=0 ; i<TEST_LOOP ; i++) {
			one_core_sum[i] += ((float)a[i] + (float)b[i] + 0.9876);
		}
		DBG_LOG("start 1_core computing -\n");

#ifdef NEED_OPENMP
		DBG_LOG("start %d_core computing +\n", omp_get_num_procs());
		#pragma omp parallel for
		for (int i=0 ; i<TEST_LOOP ; i++) {
			multi_core_sum[i] += ((float)a[i] + (float)b[i] + 0.9876);
		}
		#pragma omp barrier
		#pragma omp master
		DBG_LOG("start %d_core computing -\n", omp_get_num_procs());

		for (int i=0 ; i<TEST_LOOP ; i++) {
			if (one_core_sum[i] != multi_core_sum[i]) {
				DBG_LOG("$$$$$$$$$$$$$$$$$$$$\n");
				DBG_LOG("result not the same at %d, a, b, 1_core, %d_core[%u %u %f %f]\n",
					test_cnt, omp_get_num_procs(), (uint32_t)a[i], (uint32_t)b[i], one_core_sum[i], multi_core_sum[i]);
				param[2].value.a=0xdead;
				return;
			}
		}
#endif
    }
}

void test_omp_do_init(uint8_t *source_a, uint8_t *source_b, float *source_one_core_sum, float *source_multi_core_sum, MTEEC_PARAM *param)
{
	uint8_t *a, *b;
	float *one_core_sum, *multi_core_sum;

	if (source_a!=NULL && source_b!=NULL && source_one_core_sum!=NULL && source_multi_core_sum != NULL ) {
		a = source_a; b = source_b; one_core_sum = source_one_core_sum; multi_core_sum = source_multi_core_sum;
	} else {
		param[2].value.a=0xdeac;
		DBG_LOG("not get source_a fail\n");
		return;
	}

	DBG_LOG("init a, b data +\n");
	srand(MTEE_GetSystemCnt());
	for (int i=0 ; i<TEST_LOOP ; i++) {
		a[i] = rand()%3U;
		b[i] = rand()%3U;
		one_core_sum[i] = 0;
		multi_core_sum[i] = 0;
		//if (i>100 && i<110) DBG_LOG("[%u %u]\n", (uint32_t)a[i], (uint32_t)b[i]);
	}
	DBG_LOG("init a, b data -\n");
}

void test_omp_result(uint8_t *source_a, uint8_t *source_b, float *source_one_core_sum, float *source_multi_core_sum, MTEEC_PARAM *param)
{
	uint8_t *a, *b;
	float *one_core_sum, *multi_core_sum;

	if (source_a!=NULL && source_b!=NULL && source_one_core_sum!=NULL && source_multi_core_sum != NULL ) {
		a = source_a; b = source_b; one_core_sum = source_one_core_sum; multi_core_sum = source_multi_core_sum;
	} else {
		param[2].value.a=0xdeac;
		DBG_LOG("not get source_a fail\n");
		return;
	}

#ifdef NEED_OPENMP
	for (int i=0 ; i<TEST_LOOP ; i++) {
		if (one_core_sum[i] != multi_core_sum[i]) {
			DBG_LOG("$$$$$$$$$$$$$$$$$$$$\n");
			DBG_LOG("result not the same, a, b, 1_core, %d_core[%u %u %f %f], offset=%f\n",
				omp_get_num_procs(), (uint32_t)a[i], (uint32_t)b[i], one_core_sum[i], multi_core_sum[i], (float)i/10.0);
			param[2].value.a=0xdead;
			return;
		}
	}
#else
	for (int i=0 ; i<TEST_LOOP ; i++) {
		if (one_core_sum[i] != multi_core_sum[i]) {
			DBG_LOG("$$$$$$$$$$$$$$$$$$$$\n");
			DBG_LOG("result not the same, a, b, 1_core, 2_core[%u %u %f %f], offset=%f\n",
				(uint32_t)a[i], (uint32_t)b[i], one_core_sum[i], multi_core_sum[i], (float)i/10.0);
			param[2].value.a=0xdead;
			return;
		}
	}
#endif
}

void test_omp_one_core(uint8_t *source_a, uint8_t *source_b, float *source_one_core_sum, float *source_multi_core_sum, MTEEC_PARAM *param)
{
	uint8_t *a, *b;
	float *one_core_sum, *multi_core_sum;
	float local_offset;

	if (source_a!=NULL && source_b!=NULL && source_one_core_sum!=NULL && source_multi_core_sum != NULL ) {
		a = source_a; b = source_b; one_core_sum = source_one_core_sum; multi_core_sum = source_multi_core_sum;
	} else {
		param[2].value.a=0xdeac;
		DBG_LOG("not get source_a fail\n");
		return;
	}

	DBG_LOG("start 1_core computing +\n");
	for (int i=0 ; i<TEST_LOOP ; i++) {
		local_offset = (float)i/10.0;
		one_core_sum[i] += ((float)a[i] + (float)b[i] + local_offset);
	}
	DBG_LOG("start 1_core computing -\n");
}

void test_omp_multi_core(uint8_t *source_a, uint8_t *source_b, float *source_one_core_sum, float *source_multi_core_sum, MTEEC_PARAM *param)
{
	uint8_t *a, *b;
	float *one_core_sum, *multi_core_sum;
	float local_offset;

	if (source_a!=NULL && source_b!=NULL && source_one_core_sum!=NULL && source_multi_core_sum != NULL ) {
		a = source_a; b = source_b; one_core_sum = source_one_core_sum; multi_core_sum = source_multi_core_sum;
	} else {
		param[2].value.a=0xdeac;
		DBG_LOG("not get source_a fail\n");
		return;
	}

#ifdef NEED_OPENMP
	DBG_LOG("start %d_core computing +\n", omp_get_num_procs());
	#pragma omp parallel for
	for (int i=0 ; i<TEST_LOOP ; i++) {
		local_offset = (float)i/10.0;
		multi_core_sum[i] += ((float)a[i] + (float)b[i] + local_offset);
	}
	#pragma omp barrier
	DBG_LOG("start %d_core computing -\n", omp_get_num_procs());
#endif
}

typedef struct
{
	uint8_t *source_a;
	uint8_t *source_b;
	float *source_multi_core_sum;
} helper_param_t;

static void computing_helper1(void *arg)
{
	uint8_t *a, *b;
	float *multi_core_sum;
	helper_param_t *my_param;
	my_param = (helper_param_t *)arg;
	float local_offset;

	//DBG_LOG("%s %p %p %p\n", __func__, my_param->source_a, my_param->source_b, my_param->source_multi_core_sum);

	if (my_param->source_a!=NULL && my_param->source_b!=NULL && my_param->source_multi_core_sum != NULL ) {
		a = my_param->source_a; b = my_param->source_b; multi_core_sum = my_param->source_multi_core_sum;
	} else {
		mtee_exit(0);
	}

	for (int i=(TEST_LOOP/2) ; i<TEST_LOOP ; i++) {
		local_offset = (float)i/10.0;
		multi_core_sum[i] += ((float)a[i] + (float)b[i] + local_offset);
	}
	//my2_ioctl_send("MULTICORE_UP", MTEE_MULTICORE_UP, NULL);
	mtee_exit(0);
}

void test_two_thread1(uint8_t *source_a, uint8_t *source_b, float *source_one_core_sum, float *source_multi_core_sum, MTEEC_PARAM *param)
{
	uint8_t *a, *b;
	float *one_core_sum, *multi_core_sum;
	float local_offset;
	helper_param_t helper_param;
	int rc;
	MTEE_THREAD_HANDLE helper1_thread_handle = 0;

	if (source_a!=NULL && source_b!=NULL && source_one_core_sum!=NULL && source_multi_core_sum != NULL ) {
		a = source_a; b = source_b; one_core_sum = source_one_core_sum; multi_core_sum = source_multi_core_sum;
		helper_param.source_a = source_a;
		helper_param.source_b = source_b;
		helper_param.source_multi_core_sum = source_multi_core_sum;
		DBG_LOG("%s %p %p %p\n", __func__, source_a, source_b, source_multi_core_sum);
	} else {
		param[2].value.a=0xdeac;
		DBG_LOG("not get source_a fail\n");
		return;
	}

	DBG_LOG("start two_thread1 computing +\n");
	rc = MTEE_CreateThreadWithCPU(&helper1_thread_handle, (void *)computing_helper1, &helper_param, DEFAULT_PRIORITY, 5, 1);
	//DBG_LOG("new thread(0x%llx) rc=%d\n", helper_thread_handle, rc);
	for (int i=0 ; i<(TEST_LOOP/2) ; i++) {
		local_offset = (float)i/10.0;
		multi_core_sum[i] += ((float)a[i] + (float)b[i] + local_offset);
	}
	MTEE_JoinThread(helper1_thread_handle);
	DBG_LOG("start two_thread1 computing -\n");
}

MTEE_SEMAPHORE *help2_sema_go;
MTEE_SEMAPHORE *help2_sema_done;
bool computing_helper2_exit = true;
MTEE_THREAD_HANDLE helper2_thread_handle = 0;

static void __attribute__((constructor))
test_omp_init (void)
{
	printf("constructor: %s \n", __func__);

	help2_sema_go = MTEE_CreateSemaphore(0);
	if (help2_sema_go == NULL) {
		printf("[ERROR] help2_sema_go create fail \n");
	}

	help2_sema_done = MTEE_CreateSemaphore(0);
	if (help2_sema_done == NULL) {
		printf("[ERROR] help2_sema_done create fail \n");
	}
}

static void __attribute__((destructor))
test_omp_exit (void)
{
	printf("destructor: %s \n", __func__);

	if (help2_sema_go != NULL) {
		MTEE_DestroySemaphore(help2_sema_go);
	}

	if (help2_sema_done != NULL) {
		MTEE_DestroySemaphore(help2_sema_done);
	}
}

static void computing_helper2(void *arg)
{
	uint8_t *a, *b;
	float *multi_core_sum;
	helper_param_t *my_param;
	my_param = (helper_param_t *)arg;
	float local_offset;

	//DBG_LOG("%s %p %p %p\n", __func__, my_param->source_a, my_param->source_b, my_param->source_multi_core_sum);

	if (my_param->source_a!=NULL && my_param->source_b!=NULL && my_param->source_multi_core_sum != NULL ) {
		a = my_param->source_a; b = my_param->source_b; multi_core_sum = my_param->source_multi_core_sum;
	} else {
		mtee_exit(0);
	}

	while (1)
	{
		MTEE_DownSemaphore(help2_sema_go);
		if (computing_helper2_exit) {
			DBG_LOG("%s call mtee_exit\n", __func__);
			mtee_exit(0);;
		}
		for (int i=(TEST_LOOP/2) ; i<TEST_LOOP ; i++) {
			local_offset = (float)i/10.0;
			multi_core_sum[i] += ((float)a[i] + (float)b[i] + local_offset);
		}
		MTEE_UpSemaphore(help2_sema_done);
	}
	mtee_exit(0);
}

void test_create_thread_for_wait(uint8_t *source_a, uint8_t *source_b, float *source_one_core_sum, float *source_multi_core_sum, MTEEC_PARAM *param)
{
	uint8_t *a, *b;
	float *one_core_sum, *multi_core_sum;
	float local_offset;
	helper_param_t helper_param;
	int rc;

	if (source_a!=NULL && source_b!=NULL && source_one_core_sum!=NULL && source_multi_core_sum != NULL ) {
		a = source_a; b = source_b; one_core_sum = source_one_core_sum; multi_core_sum = source_multi_core_sum;
		helper_param.source_a = source_a;
		helper_param.source_b = source_b;
		helper_param.source_multi_core_sum = source_multi_core_sum;
		DBG_LOG("%s %p %p %p\n", __func__, source_a, source_b, source_multi_core_sum);
	} else {
		param[2].value.a=0xdeac;
		DBG_LOG("not get source_a fail\n");
		return;
	}

	computing_helper2_exit = false;
	rc = MTEE_CreateGZThread(&helper2_thread_handle, (void *)computing_helper2, &helper_param, DEFAULT_PRIORITY);
	DBG_LOG("%s-\n", __func__);
}

void test_create_thread_for_exit(uint8_t *source_a, uint8_t *source_b, float *source_one_core_sum, float *source_multi_core_sum, MTEEC_PARAM *param)
{
	uint8_t *a, *b;
	float *one_core_sum, *multi_core_sum;
	int rc;

	if (source_a!=NULL && source_b!=NULL && source_one_core_sum!=NULL && source_multi_core_sum != NULL ) {
		a = source_a; b = source_b; one_core_sum = source_one_core_sum; multi_core_sum = source_multi_core_sum;
	} else {
		param[2].value.a=0xdeac;
		DBG_LOG("not get source_a fail\n");
		return;
	}

	computing_helper2_exit = true;
	MTEE_UpSemaphore(help2_sema_go);
	MTEE_JoinThread(helper2_thread_handle);
}

void test_two_thread2(uint8_t *source_a, uint8_t *source_b, float *source_one_core_sum, float *source_multi_core_sum, MTEEC_PARAM *param)
{
	uint8_t *a, *b;
	float *one_core_sum, *multi_core_sum;
	float local_offset;
	int rc;

	if (source_a!=NULL && source_b!=NULL && source_one_core_sum!=NULL && source_multi_core_sum != NULL ) {
		a = source_a; b = source_b; one_core_sum = source_one_core_sum; multi_core_sum = source_multi_core_sum;
		DBG_LOG("%s %p %p %p\n", __func__, source_a, source_b, source_multi_core_sum);
	} else {
		param[2].value.a=0xdeac;
		DBG_LOG("not get source_a fail\n");
		return;
	}

	DBG_LOG("start two_thread2 computing +\n");
	MTEE_UpSemaphoreThread(help2_sema_go, 6, 1);
	for (int i=0 ; i<(TEST_LOOP/2) ; i++) {
		local_offset = (float)i/10.0;
		multi_core_sum[i] += ((float)a[i] + (float)b[i] + local_offset);
	}
	MTEE_DownSemaphore(help2_sema_done);
	DBG_LOG("start two_thread2 computing -\n");
}

static void test_QueryShareMem(MTEEC_PARAM *param)
{
	void *CA_HA1_shm_va;
	MTEE_SHAREDMEM_HANDLE HA1_CAHA1_shm_handle;
	MTEE_SHAREDMEM_PARAM CA_HA1_shm_info;	/*shm*/

	param[3].value.a = 0xcc;
	int ret;

	if ( param[0].value.a !=0 )
	{
		HA1_CAHA1_shm_handle = param[0].value.a;
		ret = TA_Mem_QuerySharedmem(HA1_CAHA1_shm_handle, &CA_HA1_shm_info);
		if (ret) {
			ERR_LOG("TA_Mem_QuerySharedmem error.[ret=%d], return %s\n", ret, __func__);
			return;
		} else {
			CA_HA1_shm_va = (void *) CA_HA1_shm_info.buffer;
			DBG_LOG("query share mem(%p) size(%d) %s\n", CA_HA1_shm_va, CA_HA1_shm_info.size, __func__);
			if ( *(uint32_t *)CA_HA1_shm_info.buffer != param[0].value.b ) {
				DBG_LOG("not the same %u %u %s\n", *(uint32_t *)CA_HA1_shm_info.buffer, param[0].value.b, __func__);
				return;
			} else {
				*(uint32_t *)CA_HA1_shm_info.buffer = (param[0].value.b + 1);
				DBG_LOG("new=%u old=%u %s\n", *(uint32_t *)CA_HA1_shm_info.buffer, param[0].value.b, __func__);
			}
		}
	}
	param[3].value.a = 0x66;
}

void test_omp_bypass(int command, uint8_t *content_a, uint8_t *content_b, float *content_one_core_sum, float *content_two_core_sum, MTEEC_PARAM *param)
{
	switch (command) {
		case TZCMD_TEST_OMP_BASIC:
			test_omp_basic();
			break;

		case TZCMD_TEST_OMP_UT:
			test_omp_ut(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;

		case TZCMD_TEST_OMP_DO_INIT:
			test_omp_do_init(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;

		case TZCMD_TEST_OMP_MULTI_CORE:
			test_omp_multi_core(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;

		case TZCMD_TEST_OMP_ONE_CORE:
			test_omp_one_core(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;

		case TZCMD_TEST_OMP_RESULT:
			test_omp_result(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;

		case TZCMD_TEST_OMP_102:
			DBG_LOG("try memcpy for data abort\n");
			memset(NULL, 'e', 1024);
			break;
		case TZCMD_TEST_OMP_103:
			DBG_LOG("try exit HA\n");
			exit(0);
			break;
		case TZCMD_TEST_OMP_104:
		case TZCMD_TEST_OMP_109:
			test_QueryShareMem(param);
			break;
		case 110:
			test_two_thread1(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;
		case 111:
			test_omp_one_core(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_two_thread1(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_result(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_one_core(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_two_thread1(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_result(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_one_core(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_two_thread1(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_result(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;
		case 112:
			test_omp_one_core(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_two_thread2(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_result(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_one_core(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_two_thread2(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_result(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_one_core(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_two_thread2(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			test_omp_result(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;
		case 115:
			test_create_thread_for_wait(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;
		case 116:
			test_two_thread2(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;
		case 117:
			test_create_thread_for_exit(content_a, content_b, content_one_core_sum, content_two_core_sum, param);
			break;
		default:
			DBG_LOG("%s not support %d cmd\n", __func__, command);
			break;
	}

	return;
}


