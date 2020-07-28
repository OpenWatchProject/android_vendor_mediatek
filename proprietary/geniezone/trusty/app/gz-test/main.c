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
#ifdef MULTICORE_TEST
#include <stdlib.h>
#endif
#include <err.h>
#include <trusty_std.h>
#include <mtee_server.h>
#include <tz_private/ta_sys_mem.h>
#include <tz_private/sys_ipc.h>
#include <tz_private/log.h>
#include <tz_cross/ta_test.h>

#include <lib/mtee/mtee_sys.h>
#include <lib/mtee/mtee_location.h>

#define LOG_TAG "GZ TEST"

static void gz_test_init (tipc_srv_t *srv);
static void gz_test_port (uint32_t chan, tipc_srv_t *srv);
static void gz_test_disconnect (uint32_t chan, tipc_srv_t *srv);
static int gz_test_handler (int session, int command, uint32_t paramTypes, MTEEC_PARAM *param);

static void ree_test(int session);
static void gp_param_test(MTEEC_PARAM *param);
static void shared_mem_test(MTEEC_PARAM *param, uint32_t cmd);

extern void test_rtc(MTEEC_PARAM *param);
void test_CXX_main (void);

void test_storage_bypass(int command);

#ifdef MULTICORE_TEST
void test_omp_bypass(int command,
					uint8_t *source_a,
					uint8_t *source_b,
					float *source_one_core_sum,
					float *source_multi_core_sum,
					MTEEC_PARAM *param);
uint8_t *content_a;
uint8_t *content_b;
float *content_one_core_sum;
float *content_multi_core_sum;
#include "gz-test_common.h"
#endif

void test_shared_mem_HAs_alloc(MTEEC_PARAM *param);
void test_shared_mem_ha_ta_setup(MTEEC_PARAM *param);
void test_shared_mem_HAs_copy(MTEEC_PARAM *param);
void test_shared_mem_HAs_free(MTEEC_PARAM *param);

#define MAX_PORT_BUF_SIZE 4096
#define IPC_PORT_ALLOW_ALL  (  IPC_PORT_ALLOW_NS_CONNECT \
							 | IPC_PORT_ALLOW_TA_CONNECT \
							)

#define SRV_NAME(name)   "com.mediatek.geniezone" name

// services register table
static const struct tipc_srv _services[] =
{
	{
		.name = SRV_NAME(".test"),
		.msg_num = 8,
		.msg_size = MAX_PORT_BUF_SIZE,
		.port_flags = IPC_PORT_ALLOW_ALL,
		.init_handler = gz_test_init,
		.port_connected_handler = gz_test_port,
		.disc_handler = gz_test_disconnect,
		.service_handler = gz_test_handler
	}
};

// init handler
static void gz_test_init (tipc_srv_t *srv)
{
	DBG_LOG("[%s] <%s> service init\n", __FUNCTION__, srv->name);
}

// port connnected handler
static void gz_test_port (uint32_t chan, tipc_srv_t *srv)
{
	DBG_LOG("[%s] <%s> service port connected (channel = %d)\n", __FUNCTION__, srv->name, chan);
}

static void gz_test_disconnect (uint32_t chan, tipc_srv_t *srv)
{
	DBG_LOG("[%s] <%s> disconnected (channel = %d)\n", __FUNCTION__, srv->name, chan);
}

#define SW_TIMER_MAX 3 // limited by APP thread number

typedef void *(*MTEE_SwTimerIsrFunc)(void *args);


typedef struct sw_timer_info {
	int id;
	uint64_t delay;
	int valid;
	MTEE_THREAD_HANDLE thread;
	MTEE_SwTimerIsrFunc isrFunc;
} sw_timer_info_t;


static sw_timer_info_t sw_timer_info_pool[SW_TIMER_MAX];

void _SwTimerIsrFunc (sw_timer_info_t *info)
{
	DBG_LOG("------------------------->>>>>>>>>>>>>>>>_TimerIsrThread[%d]: delay = %lld wake-up!!!\n", info->id, info->delay);
}

void _TimerIsrThread (sw_timer_info_t *info)
{
	DBG_LOG("------------------------->_TimerIsrThread[%d]: delay = %lld\n", info->id, info->delay);

	nanosleep(0, 0, info->delay);

	info->isrFunc(info);

	info->valid = 0; // NOTE: clear valid here, must check if create thread success

	mtee_exit(0);
}

static void sw_timer_init (void)
{
	int i;

	for (i = 0; i < SW_TIMER_MAX; i++) {
		memset(&sw_timer_info_pool[i], 0, sizeof(sw_timer_info_t));
	}
}
static int sw_timer_find_valid (void)
{
	int i;

	for (i = 0; i < SW_TIMER_MAX; i++) {
		if (sw_timer_info_pool[i].valid == 0) {
			sw_timer_info_pool[i].valid = 1;
			break;
		}
	}

	if (i == SW_TIMER_MAX)
		i = -1;

	return i;
}

static sw_timer_info_t *sw_timer_get_info(int id)
{
	return &sw_timer_info_pool[id];
}

static void sw_timer_test (void)
{
	int sw_timer_id;

	sw_timer_id = sw_timer_find_valid();
	if (sw_timer_id >= 0) {
		sw_timer_get_info(sw_timer_id)->id = sw_timer_id;
		sw_timer_get_info(sw_timer_id)->delay = 5ULL * 1000 * 1000 * 1000;
		sw_timer_get_info(sw_timer_id)->isrFunc = (MTEE_SwTimerIsrFunc) _SwTimerIsrFunc;
		DBG_LOG ("set sw timer [%d], delay = %lld\n", sw_timer_id, sw_timer_get_info(sw_timer_id)->delay);
		MTEE_CreateGZThread(&sw_timer_get_info(sw_timer_id)->thread, (MTEE_ThreadFunc) _TimerIsrThread, sw_timer_get_info(sw_timer_id), 0);
	}

	sw_timer_id = sw_timer_find_valid();
	if (sw_timer_id >= 0) {
		sw_timer_get_info(sw_timer_id)->id = sw_timer_id;
		sw_timer_get_info(sw_timer_id)->delay = 1ULL * 1000 * 1000 * 1000;
		sw_timer_get_info(sw_timer_id)->isrFunc = (MTEE_SwTimerIsrFunc) _SwTimerIsrFunc;
		DBG_LOG ("set sw timer [%d], delay = %lld\n", sw_timer_id, sw_timer_get_info(sw_timer_id)->delay);
		MTEE_CreateGZThread(&sw_timer_get_info(sw_timer_id)->thread, (MTEE_ThreadFunc) _TimerIsrThread, sw_timer_get_info(sw_timer_id), 0);
	}

	sw_timer_id = sw_timer_find_valid();
	if (sw_timer_id >= 0) {
		sw_timer_get_info(sw_timer_id)->id = sw_timer_id;
		sw_timer_get_info(sw_timer_id)->delay = 3ULL * 1000 * 1000 * 1000;
		sw_timer_get_info(sw_timer_id)->isrFunc = (MTEE_SwTimerIsrFunc) _SwTimerIsrFunc;
		DBG_LOG ("set sw timer [%d], delay = %lld\n", sw_timer_id, sw_timer_get_info(sw_timer_id)->delay);
		MTEE_CreateGZThread(&sw_timer_get_info(sw_timer_id)->thread, (MTEE_ThreadFunc) _TimerIsrThread, sw_timer_get_info(sw_timer_id), 0);
	}
}

extern void fptest_arch_init(uint64_t base_value);
extern int fptest_arch_check_state(uint64_t base_value);
static void vfp_test_asm(void)
{
	int corrupted_regs;

	fptest_arch_init(0);
	corrupted_regs = fptest_arch_check_state(0);
	if (corrupted_regs) {
		ERR_LOG("%s APP(%d): bad register state detected, %d registers corrupted\n",
				__func__, 0, corrupted_regs);
	} else {
		DBG_LOG("%s APP(%d): register state OK\n", __func__, 0);
	}
}

static void vfp_test_fp_sample()
{
	int expect_count = 4;

	for(int i=0;i<100;i++){
		float minl = 120.0f;
		float MIN_DET_SIZE = 12.0f;
		float minsize = 40.0f;
		float m = (float)MIN_DET_SIZE / minsize;
		minl *= m;
		//DBG_LOG("libin mtcnn m:%f  minl: %f MIN_SIZE: %f\n", m,minl,MIN_DET_SIZE);
		float factor = 0.709f;
		int factor_count = 0;

		do {
			if (minl > MIN_DET_SIZE)
			{
				if (factor_count > 0) m *= factor;


				minl *= factor;
				//DBG_LOG("libin mtcnn m:%f  minl: %f MIN_SIZE: %f\n", m,minl,MIN_DET_SIZE);
				factor_count++;
				if (minl > MIN_DET_SIZE) {
					;//DBG_LOG("continue \n");
				} else {
					//DBG_LOG("libin factor_count:%d\n", factor_count);
					break;
				}
			}
		} while(1);

		if (factor_count == expect_count) {
			DBG_LOG("%s APP(%d): compare OK %d (0x%x)\n", __func__, 0, i, expect_count);
		} else {
			ERR_LOG("%s APP(%d): compare error %d (0x%x != 0x%x)\n", __func__, 0,
					i, factor_count, expect_count);
		}
	}
}

static void vfp_test(void)
{
	vfp_test_asm();
	vfp_test_fp_sample();
}

static int gz_test_handler (int session, int command, uint32_t paramTypes, MTEEC_PARAM *param)
{
	int rc;
	mtee_info_t mtee_info;

	/*
	 * Process the incoming command.
	 * The commands are defined in tz_cross/ta_test.h
	 * Note that the commands must be synced with Linux kernel side
	 */
	switch (command) {
		case TZCMD_TEST_EXIT:
			DBG_LOG(" TA EXIT !!!!!!!!!!!!!!!!!!!!!!!\n");
			exit(0);
			break;
		case TZCMD_TEST_SYSCALL:
			DBG_LOG("=== TZCMD_TEST_SYSCALL(0x%x) ===\n", command);
			ree_test(session);
			gp_param_test(param);
			DBG_LOG("=== TZCMD_TEST_SYSCALL(0x%x) DONE ===\n", command);
			break;
		case TZCMD_SHARED_MEM_TEST:
			DBG_LOG("=== TZCMD_SHARED_MEM_TEST(0x%x) ===\n", command);
			shared_mem_test(param, command);
			DBG_LOG("=== TZCMD_SHARED_MEM_TEST(0x%x) DONE ===\n", command);
			break;
		case TZCMD_TEST_CXXLIB:
			test_CXX_main();
			break;

		case TZCMD_TEST_RTC:
			test_rtc(param);
			break;

		case TZCMD_TEST_VERSION:
			MTEE_GetVersion(&mtee_info);
			DBG_LOG("MTEE version = %s\n", mtee_info.version);
			if (1) {
				uint8_t uuid[16];
				MTEE_GetUUID(uuid);
				DBG_LOG("uuid\n");
				DBG_LOG("0x%02x 0x%02x 0x%02x 0x%02x\n", uuid[0],  uuid[1],  uuid[2],  uuid[3]);
				DBG_LOG("0x%02x 0x%02x 0x%02x 0x%02x\n", uuid[4],  uuid[5],  uuid[6],  uuid[7]);
				DBG_LOG("0x%02x 0x%02x 0x%02x 0x%02x\n", uuid[8],  uuid[9],  uuid[10], uuid[11]);
				DBG_LOG("0x%02x 0x%02x 0x%02x 0x%02x\n", uuid[12], uuid[13], uuid[14], uuid[15]);
			}
			break;

		case TZCMD_TEST_CELLINFO:
            {
                MTEE_CellInfoList cell;
    			MTEE_GetCellInfo(&cell);
                DBG_LOG("TZCMD_TEST_CELLINFO cell1.cellInfoType = %d\n", cell.cell1.cellInfoType);
                DBG_LOG("TZCMD_TEST_CELLINFO cell1.CellInfo.gsm.mcc = %d\n", cell.cell1.CellInfo.gsm.mcc);
                DBG_LOG("TZCMD_TEST_CELLINFO cell1.CellInfo.gsm.mnc = %d\n", cell.cell1.CellInfo.gsm.mnc);
                DBG_LOG("TZCMD_TEST_CELLINFO cell1.CellInfo.gsm.lac = %d\n", cell.cell1.CellInfo.gsm.lac);
                DBG_LOG("TZCMD_TEST_CELLINFO cell1.CellInfo.gsm.cid = %d\n", cell.cell1.CellInfo.gsm.cid);
    		}
			break;

		case TZCMD_TEST_VFP:
			vfp_test();
			break;

		case TZCMD_TEST_SWTIMER:
			sw_timer_test();
			break;

		case 0x11335577:
			if(paramTypes != TZ_ParamTypes1(TZPT_MEM_OUTPUT))
				return TZ_RESULT_ERROR_BAD_FORMAT;
			MTEE_GetVersion(&mtee_info);
			strncpy(param[0].mem.buffer, mtee_info.version, param[0].mem.size);
			DBG_LOG("MTEE version = %s\n", param[0].mem.buffer);
			break;

		case 0x22446688:
			if(paramTypes != TZ_ParamTypes4(TZPT_VALUE_INOUT, TZPT_VALUE_INOUT, TZPT_VALUE_INOUT, TZPT_VALUE_INOUT))
				return TZ_RESULT_ERROR_BAD_FORMAT;
			param[0].value.b = param[0].value.a;
			param[1].value.b = param[1].value.a;
			param[2].value.b = param[2].value.a;
			param[3].value.b = param[3].value.a;
			break;

		case TZCMD_TEST_OMP_BASIC:
		case TZCMD_TEST_OMP_UT:
		case TZCMD_TEST_OMP_102:
		case TZCMD_TEST_OMP_103:
		case TZCMD_TEST_OMP_104:
		case TZCMD_TEST_OMP_DO_INIT:
		case TZCMD_TEST_OMP_MULTI_CORE:
		case TZCMD_TEST_OMP_ONE_CORE:
		case TZCMD_TEST_OMP_RESULT:
		case TZCMD_TEST_OMP_109:
		case 110:
		case 111:
		case 112:
		case 115:
		case 116:
		case 117:
#ifdef MULTICORE_TEST
			DBG_LOG("MULTICORE_TEST\n");
			test_omp_bypass(command, content_a, content_b, content_one_core_sum, content_multi_core_sum, param);
#else
			DBG_LOG("$$$$$$$$$$ NOT SUPPORT MULTICORE_TEST $$$$$$$$$$\n");
#endif
			break;

		case 200:
		case 201:
		case 202:
		case 300:
		case 301:
		case 302:
		case 310:
		case 311:
		case 312:
			test_storage_bypass(command);
			break;

		case TZCMD_TEST_SHARED_MEM_HAs_ALLOC:
			test_shared_mem_HAs_alloc(param);
			break;

		case TZCMD_TEST_SHARED_MEM_HA_TA_SETUP:
			test_shared_mem_ha_ta_setup(param);
			break;

		case TZCMD_TEST_SHARED_MEM_HAs_COPY:
			test_shared_mem_HAs_copy(param);
			break;

		case TZCMD_TEST_SHARED_MEM_HAs_FREE:
			test_shared_mem_HAs_free(param);
			break;

		case TZCMD_ABORT_TEST:
			DBG_LOG("TZCMD_ABORT_TEST(0x%x) ---\n", command);
            upanic();
			DBG_LOG("TZCMD_ABORT_TEST(0x%x) --- DONE\n", command);
			break;
		default:
			break;
	}

	return 0;
}

static void ree_test(int session)
{
	MTEEC_PARAM ree_param[4];
	uint32_t ree_paramTypes;
	int ree_command;

	ree_command = REE_SERVICE_CMD_ADD;
	ree_paramTypes = TZ_ParamTypes3(TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT, TZPT_VALUE_INPUT);
	ree_param[0].value.a = 0x12;
	ree_param[1].value.a = 0x34;
	MTEE_ReeServiceCall (session, ree_command, ree_paramTypes, ree_param);
	DBG_LOG("REE_SERVICE_CMD_ADD (0x%x): expected 0x46, result 0x%x \n", ree_command, ree_param[2].value.a);

	ree_command = REE_SERVICE_CMD_MUL;
	ree_paramTypes = TZ_ParamTypes3(TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT, TZPT_VALUE_INPUT);
	ree_param[0].value.a = 64;
	ree_param[1].value.a = 32;
	MTEE_ReeServiceCall (session, ree_command, ree_paramTypes, ree_param);
	DBG_LOG("REE_SERVICE_CMD_MUL (0x%x): expected 0x800, result 0x%x \n", ree_command, ree_param[2].value.a);
}

static char* sample_data1 = "sample data 1!!";

static int check_modify(char *buf, unsigned int len)
{
	int i;
	int rc = 0;

	for (i = 0; i < len; i++) {

		if (buf[i]!='c'){
			rc = i;
			break;
		}

		// modify value
		if ((i%3)==0) buf[i] = 'd';
		else buf[i] = 'c';
	}
	return rc;
}

static void gp_param_test(MTEEC_PARAM *param)
{
	int rc;

	DBG_LOG("gp_param_test: expected 0x1230, result 0x%x \n", param[0].value.a);

	rc = check_modify((char*)param[2].mem.buffer, 512);
	DBG_LOG("check_modify: expected 0, result %d \n", rc);

	strncpy(param[1].mem.buffer, sample_data1, param[1].mem.size);
	param[3].value.a = 99;
}

static void shared_mem_test(MTEEC_PARAM *param, uint32_t cmd)
{
	MTEE_SHAREDMEM_PARAM shm_param;
	MTEE_SHAREDMEM_HANDLE shm_handle;
	TZ_RESULT ret;
	char *va, *va2;
	int numOfPA;
	int in_size;

	//for test stat
	int i;
	int stat[2]; //0: for a; 1: for b;

	for (i=0;i<2;i++) {
		stat[i] = 0;
	}

	if (!param) return;

	shm_handle = (MTEE_SHAREDMEM_HANDLE) param[0].value.a;
	numOfPA =  param[1].value.a;
	in_size = param[1].value.b;

	// Query first time
	ret = TA_Mem_QuerySharedmem(shm_handle, &shm_param);
	if (ret) {
		ERR_LOG("TA_Mem_QuerySharedmem error. [ret=%d]\n", ret);
		return;
	}

	va = (char *) shm_param.buffer;
	if (!va) {
		ERR_LOG("error: shared memory va is null \n");
		return;
	}

	// Query second time
	ret = TA_Mem_QuerySharedmem(shm_handle, &shm_param);
	if (ret) {
		ERR_LOG("TA_Mem_QuerySharedmem error. [ret=%d]\n", ret);
		return;
	}

	va2 = (char *) shm_param.buffer;
	if (!va2) {
		ERR_LOG("error: shared memory va2 is null \n");
		return;
	}

	// check if va == va2
	if(va != va2) {
		ERR_LOG("error: va != va2 \n");
		return;
	}

	//check result by statistic char counts (recommended)
	for (i=0; i< (in_size); i++) {
		if (va[i] == 'a') {
			stat[0] ++;
		} else if (va[i] == 'b') {
			stat[1] ++;
		}
	}

	DBG_LOG("[%s]====> output string in GZ #of [a] = %5d \n", __FUNCTION__ , stat[0]);
	DBG_LOG("[%s]====> output string in GZ #of [b] = %5d \n", __FUNCTION__ , stat[1]);

	//update str: a --> d
	for (i=0; i< (in_size); i++) {
		if (va[i] == 'a') {
			va[i] = 'b';
		}
	}

	//init
	for (i=0;i<2;i++) {
		stat[i] = 0;
	}

	for (i=0; i< (in_size); i++) {
		if (va[i] == 'a') {
			stat[0] ++;
		} else if (va[i] == 'b') {
			stat[1] ++;
		}
	}

	DBG_LOG("[main.c]:[%s]====> output string in GZ #of [a] = %5d \n", __FUNCTION__ , stat[0]);
	DBG_LOG("[main.c]:[%s]====> output string in GZ #of [b] = %5d \n", __FUNCTION__ , stat[1]);

}

// IPC message buffer
static uint8_t msg_buf[MAX_PORT_BUF_SIZE];

// service states
static struct tipc_srv_state _srv_states[countof(_services)] = {
	[0 ... (countof(_services) - 1)] = {
		.port = INVALID_IPC_HANDLE,
	},
};

#ifdef MULTICORE_TEST
int pre_allocate_mem()
{
	content_a = NULL;
	content_b = NULL;
	content_one_core_sum = NULL;
	content_multi_core_sum = NULL;

	DBG_LOG("try a malloc %u\n", MALLOC_SIZE*sizeof(uint8_t));
	content_a = (uint8_t *)malloc(MALLOC_SIZE*sizeof(uint8_t));
	if (content_a==NULL) {
		DBG_LOG("malloc a fail\n");
		return -1;
	}
	DBG_LOG("try b malloc %u\n", MALLOC_SIZE*sizeof(uint8_t));
	content_b = (uint8_t *)malloc(MALLOC_SIZE*sizeof(uint8_t));
	if (content_b==NULL) {
		free(content_a);
		DBG_LOG("malloc b fail\n");
		return -1;
	}
	DBG_LOG("try one core malloc %u\n", MALLOC_SIZE*sizeof(float));
	content_one_core_sum = (float *)malloc(MALLOC_SIZE*sizeof(float));
	if (content_one_core_sum==NULL) {
		free(content_a);
		free(content_b);
		DBG_LOG("malloc one_core_sum fail\n");
		return -1;
	}
	DBG_LOG("try multi core malloc %u\n", MALLOC_SIZE*sizeof(float));
	content_multi_core_sum = (float *)malloc(MALLOC_SIZE*sizeof(float));
	if (content_multi_core_sum==NULL) {
		free(content_a);
		free(content_b);
		free(content_one_core_sum);
		DBG_LOG("malloc multi_core_sum fail\n");
		return -1;
	}
}
#endif

int main(void)
{
	int rc;
	uevent_t uevent;

	/* Initialize service */
	rc = init_services(_services, countof(_services), _srv_states);
	if (rc != NO_ERROR ) {
		ERR_LOG("Failed (%d) to init service", rc);
		kill_services(&_services, countof(_services), _srv_states);
		return -1;
	}

#ifdef MULTICORE_TEST
	pre_allocate_mem();
	srand(MTEE_GetSystemCnt());
#endif

	/* handle events */
	while (1) {
		uevent.handle = INVALID_IPC_HANDLE;
		uevent.event  = 0;
		uevent.cookie = NULL;
		rc = wait_any(&uevent, -1);
		if (rc < 0) {
			ERR_LOG("wait_any failed (%d)", rc);
#ifdef OMP_PERFORMANCE
			nanosleep(0, 0, 1000);
#endif
			continue;
		}
		if (rc == NO_ERROR) { /* got an event */
			dispatch_event(&uevent, msg_buf, sizeof(msg_buf));
		}
	}

	/* Terminate service (DO NOT MODIFY) */
	kill_services(_services, countof(_services), _srv_states);
	return 0;
}
