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
#include <lib/mtee/mtee_sapu_happ.h>
#include <tz_private/ta_sys_sapu.h>
#include <tz_private/ta_sys_mem.h>
#include <tz_private/sys_ipc.h>
#include <tz_private/log.h>
#include <tz_cross/ta_test.h>

#ifdef SUPPORT_sDSP_0
extern unsigned char vpu_main_sDSP_0[];
extern unsigned int vpu_main_sDSP_0_len;
#endif
#ifdef SUPPORT_sDSP_1
extern unsigned char vpu_main_sDSP_1[];
extern unsigned int vpu_main_sDSP_1_len;
#endif
#include <lib/mtee/mtee_sys.h>

#define LOG_TAG "FOD"

static void fod_sample_init (tipc_srv_t *srv);
static void fod_sample_port (uint32_t chan, tipc_srv_t *srv);
static void fod_sample_disconnect (uint32_t chan, tipc_srv_t *srv);
static int fod_sample_handler (int session, int command, uint32_t paramTypes, MTEEC_PARAM *param);

MTEE_SEMAPHORE *help2_sema_go;
MTEE_SEMAPHORE *help2_sema_done;
MTEE_THREAD_HANDLE helper2_thread_handle = 0;

MTEE_SEMAPHORE *fork_thread_sema_go;
MTEE_SEMAPHORE *fork_thread_sema_done;
MTEE_THREAD_HANDLE fork_thread_handle = 0;

static void __attribute__((constructor))
fod_sample_constructor (void)
{
	DBG_LOG("%s\n", __func__);

	help2_sema_go = MTEE_CreateSemaphore(0);
	if (help2_sema_go == NULL) {
		ERR_LOG("help2_sema_go create fail \n");
	}

	help2_sema_done = MTEE_CreateSemaphore(0);
	if (help2_sema_done == NULL) {
		ERR_LOG("help2_sema_done create fail \n");
	}

	fork_thread_sema_go = MTEE_CreateSemaphore(0);
	if (fork_thread_sema_go == NULL) {
		ERR_LOG("fork_thread_sema_go create fail \n");
	}

	fork_thread_sema_done = MTEE_CreateSemaphore(0);
	if (fork_thread_sema_done == NULL) {
		ERR_LOG("[fork_thread_sema_done create fail \n");
	}
}

static void __attribute__((destructor))
fod_sample_destructor (void)
{
	DBG_LOG("%s\n", __func__);

	if (help2_sema_go != NULL) {
		MTEE_DestroySemaphore(help2_sema_go);
	}

	if (help2_sema_done != NULL) {
		MTEE_DestroySemaphore(help2_sema_done);
	}

	if (fork_thread_sema_go != NULL) {
		MTEE_DestroySemaphore(fork_thread_sema_go);
	}

	if (fork_thread_sema_done != NULL) {
		MTEE_DestroySemaphore(fork_thread_sema_done);
	}
}

#define MAX_PORT_BUF_SIZE 4096
#define IPC_PORT_ALLOW_ALL  (  IPC_PORT_ALLOW_NS_CONNECT \
							 | IPC_PORT_ALLOW_TA_CONNECT \
							)

#define SRV_NAME(name)   "com.mediatek.gz" name

// services register table
static const struct tipc_srv _services[] =
{
	{
		.name = SRV_NAME(".fod_sample"),
		.msg_num = 8,
		.msg_size = MAX_PORT_BUF_SIZE,
		.port_flags = IPC_PORT_ALLOW_ALL,
		.init_handler = (void *)fod_sample_init,
		.port_connected_handler = (void *)fod_sample_port,
		.disc_handler = (void *)fod_sample_disconnect,
		.service_handler = (void *)fod_sample_handler,
	}
};

static void helper2(void *arg);
static void fork_thread_entry(void *arg);
// init handler
static void fod_sample_init (tipc_srv_t *srv)
{
	int rc=0;
	rc = MTEE_CreateGZThread(&helper2_thread_handle, (void *)helper2, NULL, DEFAULT_PRIORITY);
	DBG_LOG("[%s] <%s> service init(%d)\n", __FUNCTION__, srv->name, rc);
}

// port connnected handler
static void fod_sample_port (uint32_t chan, tipc_srv_t *srv)
{
	DBG_LOG("[%s] <%s> service port connected (channel = %d)\n", __FUNCTION__, srv->name, chan);
}

static void fod_sample_disconnect (uint32_t chan, tipc_srv_t *srv)
{
	DBG_LOG("[%s] <%s> disconnected (channel = %d)\n", __FUNCTION__, srv->name, chan);
}


int need_exit = 0;

void *sdsp_mtee_shm_ptr = NULL;
uint32_t sdsp_mtee_shm_size = 0;
void *sdsp_tee_shm_ptr = NULL;
uint32_t sdsp_tee_shm_size = 0;

#define VPU1_DUMP (1<<0)
#define VPU2_DUMP (1<<1)

char *_src[2];
char *_dst[2];
size_t _copysize[2];
size_t _src_size[2];
vpu_prop_t *vpu_prop[2] = {NULL, NULL};
vpu_buffer_t *vpu0_buffer = NULL;
vpu_buffer_t *vpu1_buffer = NULL;
uint32_t data_len = 0;

uint32_t VPU0_BUF_OFFSET	= 0;
uint32_t VPU0_PROP_OFFSET	= 0;
uint32_t VPU1_BUF_OFFSET	= 0;
uint32_t VPU1_PROP_OFFSET	= 0;

static void do_my_memcpy(char *src, uint32_t src_len, char *dst, uint32_t dst_len, vpu_prop_t *vpu_prop)
{
	uint32_t i, j;
	if (vpu_prop->cmd!=0x168) {
		ERR_LOG("cmd not expected 0x168(%u)\n", vpu_prop->cmd);
		vpu_prop->result = 87;
	} else if (src_len!=dst_len) {
		ERR_LOG("size not the same %u %u\n", src_len, dst_len);
		vpu_prop->result = 89;
	} else {
		for (j=0 ; j<100 ; j++) {
			for(i=0; i<dst_len; i++) {
				dst[i] = src[i];
			}
		}
		vpu_prop->result = 100;
	}
}

static void helper1(void *arg)
{
	//DBG_LOG("[%s] help~~~~~~(start) \n",__func__);
	do_my_memcpy(_src[1], _src_size[1], _dst[1], _copysize[1], vpu_prop[1]);
	//DBG_LOG("[%s] help~~~~~~(end), result(%u)\n", __func__, vpu_prop[1]->result);
	mtee_exit(0);
}

static void helper2(void *arg)
{
	while (1)
	{
		MTEE_DownSemaphore(help2_sema_go);
		do_my_memcpy(_src[1], _src_size[1], _dst[1], _copysize[1], vpu_prop[1]);
		MTEE_UpSemaphore(help2_sema_done);
	}
	mtee_exit(0);
}

static void fork_thread_entry(void *arg)
{
	int rc;
	DBG_LOG("%s\n", __func__);
	while (1)
	{
		MTEE_DownSemaphore(fork_thread_sema_go);
		vpu_prop[1]->cmd = 0x16c;
		vpu_prop[1]->result = 123;
		vpu_prop[1]->count = 200*1000*1000;
		vpu1_buffer->port_id = 1;
		vpu1_buffer->plane_count = 1;
		vpu1_buffer->planes[0].length = data_len-1024;
		vpu1_buffer->planes[0].ptr = TEE_VPU_SHM_MVA+1024;
		MTEE_CleanInvalDcacheRange((unsigned long)vpu1_buffer, 4096);
		rc = SAPU_SDSP_Core2_Run(VPU1_BUF_OFFSET, 1, VPU1_PROP_OFFSET, sizeof(vpu_prop_t), 0, NULL);
		if (rc) {
			ERR_LOG("SAPU_SDSP_Core2_Run fail rc(%d)\n", rc);
		}
		vpu_prop[1]->cmd = 0x16c;
		vpu_prop[1]->result = 123;
		vpu_prop[1]->count = 400*1000*1000;
		vpu1_buffer->port_id = 1;
		vpu1_buffer->plane_count = 1;
		vpu1_buffer->planes[0].length = data_len-2048;
		vpu1_buffer->planes[0].ptr = TEE_VPU_SHM_MVA+2048;
		MTEE_CleanInvalDcacheRange((unsigned long)vpu1_buffer, 4096);
		rc = SAPU_SDSP_Core2_Run(VPU1_BUF_OFFSET, 1, VPU1_PROP_OFFSET, sizeof(vpu_prop_t), 0, NULL);
		if (rc) {
			ERR_LOG("SAPU_SDSP_Core2_Run fail rc(%d)\n", rc);
		}
		MTEE_UpSemaphore(fork_thread_sema_done);
	}
	mtee_exit(0);
}

uint32_t test_matrix_32(void);
uint64_t test_matrix_64(void);
uint64_t matrix64_to_kernel(void);
uint64_t matrix32_to_kernel(void);

static int fod_sample_handler (int session, int command, uint32_t paramTypes, MTEEC_PARAM *param)
{
	int rc;

	uint32_t i, j;
	uint32_t sdsp1_buf_offset[2], sdsp1_buf_num[2];
	uint32_t sdsp1_prop_offset[2], sdsp1_prop_size[2];
	uint64_t basePA;
	vpu_buffer_t *vpu_buffer;
	char *src;
	char *dst;
	uint32_t VA1, VA2;
	MTEE_THREAD_HANDLE helper1_thread_handle = 0;
	MTEE_SHAREDMEM_HANDLE CAHA_shm_handle;
	MTEE_SHAREDMEM_PARAM CAHA_shm_info;	/*shm*/

	uint32_t temp32;
	uint64_t temp64;
	MTEE_Time time1;
	MTEE_Time time2;

	sapu_reg_info_t my_reg_info;

	/*
	 * Process the incoming command.
	 * The commands are defined in tz_cross/ta_test.h
	 * Note that the commands must be synced with Linux kernel side
	 */
	switch (command) {
		case 2019:
			sdsp_tee_shm_ptr = SAPU_AcquireSapuTeeShm(&sdsp_tee_shm_size);
			if (!sdsp_tee_shm_ptr) {
				ERR_LOG("sdsp_tee_shm_ptr NULL !!!!\n");
				break;
			}
			else
				DBG_LOG("sdsp_tee_shm_ptr(%p) size(0x%x)\n", sdsp_tee_shm_ptr, sdsp_tee_shm_size);

			DBG_LOG("[fod HA] vpu[0] :buf offset(0x%x) num(%u)\n", param[0].value.a, param[0].value.b);
			DBG_LOG("[fod HA] vpu[0] :prop offset(0x%x) size(%u)\n", param[1].value.a, param[1].value.b);

			DBG_LOG("[fod HA] vpu[1] :buf offset(0x%x) num(%u)\n", param[2].value.a, param[2].value.b);
			DBG_LOG("[fod HA] vpu[1] :prop offset(0x%x) size(%u)\n", param[3].value.a, param[3].value.b);

			/*VPU0*/
			vpu_prop[0] = (vpu_prop_t *)(sdsp_tee_shm_ptr + param[1].value.a);
			DBG_LOG("vpu_prop[0]->cmd = 0x%x \n", vpu_prop[0]->cmd);
			vpu0_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+param[0].value.a);


			_src[0] = sdsp_tee_shm_ptr + SAPU_vDSP_Shm_offset(vpu0_buffer[0].planes[0].ptr);
			_dst[0] = sdsp_tee_shm_ptr + SAPU_vDSP_Shm_offset(vpu0_buffer[1].planes[0].ptr);
			_src_size[0] = vpu0_buffer[0].planes[0].length;
			_copysize[0] = vpu0_buffer[1].planes[0].length;

			//vpu0 buf0 copy to vpu0 buf1
			do_my_memcpy(_src[0], _src_size[0], _dst[0], _copysize[0], vpu_prop[0]);

			/*VPU1*/
			vpu_prop[1] = (vpu_prop_t *)(sdsp_tee_shm_ptr + param[3].value.a);
			DBG_LOG("vpu_prop[1]->cmd = 0x%x \n", vpu_prop[1]->cmd);
			vpu1_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+param[2].value.a);

			_src[1] = sdsp_tee_shm_ptr + SAPU_vDSP_Shm_offset(vpu1_buffer[0].planes[0].ptr);
			_dst[1] = sdsp_tee_shm_ptr + SAPU_vDSP_Shm_offset(vpu1_buffer[1].planes[0].ptr);
			_src_size[1] = vpu1_buffer[0].planes[0].length;
			_copysize[1] = vpu1_buffer[1].planes[0].length;

			//vpu1 buf0 copy to vpu1 buf1
			do_my_memcpy(_src[1], _src_size[1], _dst[1], _copysize[1], vpu_prop[1]);

			break;

		case 100:
			sdsp_mtee_shm_ptr = SAPU_AcquireSapuMteeShm(&sdsp_mtee_shm_size);
			if (sdsp_mtee_shm_ptr!=NULL) {
				DBG_LOG("sdsp_mtee_shm_ptr(%p) size(0x%x)\n", sdsp_mtee_shm_ptr, sdsp_mtee_shm_size);
			}
			break;

		case 101:
			DBG_LOG("SAPU_ReleaseSapuMteeShm try ptr(%p), ptrAt(0x%x)\n", sdsp_mtee_shm_ptr, (uint32_t)&sdsp_mtee_shm_ptr);
			rc = SAPU_ReleaseSapuMteeShm((void **)&sdsp_mtee_shm_ptr, &sdsp_mtee_shm_size);
			if (rc!=0 || sdsp_mtee_shm_ptr!=NULL) {
				DBG_LOG("SAPU_ReleaseSapuMteeShm fail rc(%d), ptr(%p)\n", rc, sdsp_mtee_shm_ptr);
				break;
			}
			break;

		case 200:
			sdsp_tee_shm_ptr = SAPU_AcquireSapuTeeShm(&sdsp_tee_shm_size);
			if (sdsp_tee_shm_ptr!=NULL) {
				DBG_LOG("sdsp_tee_shm_ptr(%p) size(0x%x)\n", sdsp_tee_shm_ptr, sdsp_tee_shm_size);
			}
			VPU0_BUF_OFFSET		= sdsp_tee_shm_size-4096;
			VPU0_PROP_OFFSET	= sdsp_tee_shm_size-1024;
			VPU1_BUF_OFFSET		= sdsp_tee_shm_size-(4096*2);
			VPU1_PROP_OFFSET	= sdsp_tee_shm_size-4096-1024;
			break;

		case 201:
			DBG_LOG("SAPU_ReleaseSapuTeeShm try ptr(%p), ptrAt(0x%x)\n", sdsp_tee_shm_ptr, (uint32_t)&sdsp_tee_shm_ptr);
			rc = SAPU_ReleaseSapuTeeShm((void **)&sdsp_tee_shm_ptr, &sdsp_tee_shm_size);
			if (rc!=0 || sdsp_tee_shm_ptr!=NULL) {
				DBG_LOG("SAPU_ReleaseSapuTeeShm fail rc(%d), ptr(%p)\n", rc, sdsp_tee_shm_ptr);
				break;
			}
			VPU0_BUF_OFFSET	= 0;
			VPU0_PROP_OFFSET= 0;
			VPU1_BUF_OFFSET	= 0;
			VPU1_PROP_OFFSET= 0;
			break;

		case 202:
			if (sdsp_tee_shm_ptr==NULL) {
				rc = -1;
				ERR_LOG("assign data fail\n");
			}
			vpu_prop[0] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU0_PROP_OFFSET);
			vpu_prop[0]->cmd = 0x169;
			vpu_prop[0]->result = 111;
			vpu_prop[1] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU1_PROP_OFFSET);
			vpu_prop[1]->cmd = 0x169;
			vpu_prop[1]->result = 222;
			vpu0_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU0_BUF_OFFSET);
			vpu1_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU1_BUF_OFFSET);
			CAHA_shm_handle = param[0].value.a;
			data_len		= param[0].value.b;
			rc = TA_Mem_QuerySharedmem(CAHA_shm_handle, &CAHA_shm_info);
			if (data_len > CAHA_shm_info.size) return -1;
			memcpy(sdsp_tee_shm_ptr, CAHA_shm_info.buffer, data_len);
			if (1) {
				uint64_t *data = (uint64_t *)sdsp_tee_shm_ptr;
				DBG_LOG("[test patten]0x%llx 0x%llx 0x%llx 0x%llx\n", data[0], data[1], data[2], data[3]);
			}
			vpu0_buffer->port_id = 0;
			vpu0_buffer->plane_count = 1;
			vpu0_buffer->planes[0].length = data_len;
			vpu0_buffer->planes[0].ptr = TEE_VPU_SHM_MVA;
			MTEE_CleanInvalDcacheRange((unsigned long)vpu0_buffer, 4096);
			MTEE_CleanInvalDcacheRange((unsigned long)vpu1_buffer, 4096);
			MTEE_CleanInvalDcacheRange((unsigned long)sdsp_tee_shm_ptr, data_len);
			rc = SAPU_SDSP_Run(VPU0_BUF_OFFSET, 1, VPU0_PROP_OFFSET, sizeof(vpu_prop_t),
							   VPU1_BUF_OFFSET, 1, VPU1_PROP_OFFSET, sizeof(vpu_prop_t),
								0, false, NULL);	// true is SW_OP, false is sDSP
			//MTEE_CleanDcacheAll();
			if (rc) {
				DBG_LOG("SAPU_SDSP_Run(0,0) fail rc(%d)\n", rc);
				break;
			}
			param[1].value.a = vpu_prop[0]->cmd;
			param[1].value.b = vpu_prop[0]->result;
			break;

		case 203:
			if (sdsp_tee_shm_ptr==NULL) {
				rc = -1;
				ERR_LOG("assign data fail\n");
			}
			vpu_prop[0] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU0_PROP_OFFSET);
			vpu_prop[0]->cmd = 0x16b;
			vpu_prop[0]->result = 111;
			vpu_prop[1] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU1_PROP_OFFSET);
			vpu_prop[1]->cmd = 0x16b;
			vpu_prop[1]->result = 222;
			vpu0_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU0_BUF_OFFSET);
			vpu1_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU1_BUF_OFFSET);
			CAHA_shm_handle = param[0].value.a;
			data_len		= param[0].value.b;
			rc = TA_Mem_QuerySharedmem(CAHA_shm_handle, &CAHA_shm_info);
			if (data_len > CAHA_shm_info.size) return -1;
			memcpy(sdsp_tee_shm_ptr, CAHA_shm_info.buffer, data_len);
			if (1) {
				uint64_t *data = (uint64_t *)sdsp_tee_shm_ptr;
				DBG_LOG("[test patten]0x%llx 0x%llx 0x%llx 0x%llx\n", data[0], data[1], data[2], data[3]);
			}
			vpu0_buffer->port_id = 0;
			vpu0_buffer->plane_count = 1;
			vpu0_buffer->planes[0].length = data_len;
			vpu0_buffer->planes[0].ptr = TEE_VPU_SHM_MVA;
			vpu1_buffer->port_id = 1;
			vpu1_buffer->plane_count = 1;
			vpu1_buffer->planes[0].length = data_len;
			vpu1_buffer->planes[0].ptr = TEE_VPU_SHM_MVA;
			MTEE_CleanInvalDcacheRange((unsigned long)vpu0_buffer, 4096);
			MTEE_CleanInvalDcacheRange((unsigned long)vpu1_buffer, 4096);
			MTEE_CleanInvalDcacheRange((unsigned long)sdsp_tee_shm_ptr, data_len);
			rc = SAPU_SDSP_Run(VPU0_BUF_OFFSET, 1, VPU0_PROP_OFFSET, sizeof(vpu_prop_t),
							   VPU1_BUF_OFFSET, 1, VPU1_PROP_OFFSET, sizeof(vpu_prop_t),
								0, false, NULL);	// true is SW_OP, false is sDSP
			//MTEE_CleanDcacheAll();
			if (rc) {
				DBG_LOG("SAPU_SDSP_Run(0,0) fail rc(%d)\n", rc);
				break;
			}
			param[1].value.a = vpu_prop[0]->cmd;
			param[1].value.b = vpu_prop[0]->result;
			param[2].value.a = vpu_prop[1]->cmd;
			param[2].value.b = vpu_prop[1]->result;
			break;

		case 206:
			if (sdsp_tee_shm_ptr==NULL) {
				rc = -1;
				ERR_LOG("assign data fail\n");
			}
			vpu_prop[0] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU0_PROP_OFFSET);
			vpu_prop[0]->cmd = 0x16c;
			vpu_prop[0]->result = 123;
			vpu_prop[0]->count = 350*1000*1000;
			vpu_prop[1] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU1_PROP_OFFSET);
			vpu_prop[1]->cmd = 0x16c;
			vpu_prop[1]->result = 123;
			vpu_prop[1]->count = 700*1000*1000;
			vpu0_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU0_BUF_OFFSET);
			vpu1_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU1_BUF_OFFSET);
			CAHA_shm_handle = param[0].value.a;
			data_len		= param[0].value.b;
			rc = TA_Mem_QuerySharedmem(CAHA_shm_handle, &CAHA_shm_info);
			if (data_len > CAHA_shm_info.size) return -1;
			memcpy(sdsp_tee_shm_ptr, CAHA_shm_info.buffer, data_len);
			if (1) {
				uint64_t *data = (uint64_t *)sdsp_tee_shm_ptr;
				DBG_LOG("[test patten]0x%llx 0x%llx 0x%llx 0x%llx\n", data[0], data[1], data[2], data[3]);
			}
			vpu0_buffer->port_id = 0;
			vpu0_buffer->plane_count = 1;
			vpu0_buffer->planes[0].length = data_len;
			vpu0_buffer->planes[0].ptr = TEE_VPU_SHM_MVA;
			vpu1_buffer->port_id = 1;
			vpu1_buffer->plane_count = 1;
			vpu1_buffer->planes[0].length = data_len;
			vpu1_buffer->planes[0].ptr = TEE_VPU_SHM_MVA;
			MTEE_CleanInvalDcacheRange((unsigned long)vpu0_buffer, 4096);
			MTEE_CleanInvalDcacheRange((unsigned long)vpu1_buffer, 4096);
			MTEE_CleanInvalDcacheRange((unsigned long)sdsp_tee_shm_ptr, data_len);
			rc = SAPU_SDSP_Core1_Run(VPU0_BUF_OFFSET, 1, VPU0_PROP_OFFSET, sizeof(vpu_prop_t), 0, NULL);
			if (rc) {
				ERR_LOG("SAPU_SDSP_Core1_Run fail rc(%d)\n", rc);
				break;
			}
			rc = SAPU_SDSP_Core2_Run(VPU1_BUF_OFFSET, 1, VPU1_PROP_OFFSET, sizeof(vpu_prop_t), 0, NULL);
			if (rc) {
				ERR_LOG("SAPU_SDSP_Core2_Run fail rc(%d)\n", rc);
				break;
			}

			my_reg_info.sdsp1_cmd=0x16c;
			my_reg_info.sdsp1_result=0x0;
			my_reg_info.sdsp2_cmd=0x16c;
			my_reg_info.sdsp2_result=0x0;
			rc = SAPU_SDSP_Core1_Run(VPU0_BUF_OFFSET, 1, VPU0_PROP_OFFSET, sizeof(vpu_prop_t), 0, &my_reg_info);
			if (rc) {
				ERR_LOG("SAPU_SDSP_Core1_Run fail rc(%d)\n", rc);
				break;
			}
			if (!rc) {
				DBG_LOG("vpu1 cmd/result(0x%x/0x%x)\n", my_reg_info.sdsp1_cmd, my_reg_info.sdsp1_result);
			}

			my_reg_info.sdsp1_cmd=0x16c;
			my_reg_info.sdsp1_result=0x0;
			my_reg_info.sdsp2_cmd=0x16c;
			my_reg_info.sdsp2_result=0x0;
			rc = SAPU_SDSP_Core2_Run(VPU1_BUF_OFFSET, 1, VPU1_PROP_OFFSET, sizeof(vpu_prop_t), 0, &my_reg_info);
			if (rc) {
				ERR_LOG("SAPU_SDSP_Core2_Run fail rc(%d)\n", rc);
				break;
			}
			if (!rc) {
				DBG_LOG("vpu2 cmd/result(0x%x/0x%x)\n", my_reg_info.sdsp2_cmd, my_reg_info.sdsp2_result);
			}

			rc = SAPU_Log_Dump(VPU1_DUMP|VPU2_DUMP);
			if (rc) {
				ERR_LOG("VPU1_DUMP fail rc(%d)\n", rc);
				break;
			}
			break;

		case 208:
			if (sdsp_tee_shm_ptr==NULL) {
				rc = -1;
				ERR_LOG("assign data fail\n");
			}
			vpu_prop[0] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU0_PROP_OFFSET);
			vpu_prop[1] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU1_PROP_OFFSET);
			vpu0_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU0_BUF_OFFSET);
			vpu1_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU1_BUF_OFFSET);
			CAHA_shm_handle = param[0].value.a;
			data_len		= param[0].value.b;
			rc = TA_Mem_QuerySharedmem(CAHA_shm_handle, &CAHA_shm_info);
			if (data_len > CAHA_shm_info.size) return -1;
			memcpy(sdsp_tee_shm_ptr, CAHA_shm_info.buffer, data_len);
			if (1) {
				uint64_t *data = (uint64_t *)sdsp_tee_shm_ptr;
				DBG_LOG("[test patten]0x%llx 0x%llx 0x%llx 0x%llx\n", data[0], data[1], data[2], data[3]);
			}
			MTEE_CleanInvalDcacheRange((unsigned long)sdsp_tee_shm_ptr, data_len);
			MTEE_UpSemaphoreThread(fork_thread_sema_go, 6, 1);
			vpu_prop[0]->cmd = 0x16c;
			vpu_prop[0]->result = 123;
			vpu_prop[0]->count = 700*1000*1000;
			vpu0_buffer->port_id = 0;
			vpu0_buffer->plane_count = 1;
			vpu0_buffer->planes[0].length = data_len;
			vpu0_buffer->planes[0].ptr = TEE_VPU_SHM_MVA;
			MTEE_CleanInvalDcacheRange((unsigned long)vpu0_buffer, 4096);
			rc = SAPU_SDSP_Core1_Run(VPU0_BUF_OFFSET, 1, VPU0_PROP_OFFSET, sizeof(vpu_prop_t), 0, NULL);
			if (rc) {
				ERR_LOG("SAPU_SDSP_Core1_Run fail rc(%d)\n", rc);
				break;
			}
			MTEE_DownSemaphore(fork_thread_sema_done);
			rc = SAPU_Log_Dump(VPU1_DUMP|VPU2_DUMP);
			if (rc) {
				ERR_LOG("VPU_DUMP fail rc(%d)\n", rc);
				break;
			}
			break;

		case 210:
			rc = 0;
			if (fork_thread_handle!=0) {
				DBG_LOG("already fork_thread_handle\n");
				break;
			}
			rc = MTEE_CreateGZThread(&fork_thread_handle, (void *)fork_thread_entry, NULL, DEFAULT_PRIORITY);
			if (rc) {
				ERR_LOG("fork_thread_handle fail rc(%d)\n", rc);
			}
			break;

		case 209:
			if (sdsp_tee_shm_ptr==NULL) {
				rc = -1;
				ERR_LOG("assign data fail\n");
			}
			vpu_prop[0] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU0_PROP_OFFSET);
			vpu_prop[1] = (vpu_prop_t *)(sdsp_tee_shm_ptr+VPU1_PROP_OFFSET);
			vpu0_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU0_BUF_OFFSET);
			vpu1_buffer = (vpu_buffer_t *)(sdsp_tee_shm_ptr+VPU1_BUF_OFFSET);
			CAHA_shm_handle = param[0].value.a;
			data_len		= param[0].value.b;
			rc = TA_Mem_QuerySharedmem(CAHA_shm_handle, &CAHA_shm_info);
			if (data_len > CAHA_shm_info.size) return -1;
			memcpy(sdsp_tee_shm_ptr, CAHA_shm_info.buffer, data_len);
			if (1) {
				uint64_t *data = (uint64_t *)sdsp_tee_shm_ptr;
				DBG_LOG("[test patten]0x%llx 0x%llx 0x%llx 0x%llx\n", data[0], data[1], data[2], data[3]);
			}
			MTEE_CleanInvalDcacheRange((unsigned long)sdsp_tee_shm_ptr, data_len);
			MTEE_UpSemaphoreThread(fork_thread_sema_go, 6, 1);
			vpu_prop[0]->cmd = 0x16c;
			vpu_prop[0]->result = 123;
			vpu_prop[0]->count = 700*1000*1000;
			vpu0_buffer->port_id = 0;
			vpu0_buffer->plane_count = 1;
			vpu0_buffer->planes[0].length = data_len;
			vpu0_buffer->planes[0].ptr = TEE_VPU_SHM_MVA;
			MTEE_CleanInvalDcacheRange((unsigned long)vpu0_buffer, 4096);
			rc = SAPU_SDSP_Core2_Run(VPU0_BUF_OFFSET, 1, VPU0_PROP_OFFSET, sizeof(vpu_prop_t), 0, NULL);
			if (rc) {
				ERR_LOG("SAPU_SDSP_Core2_Run fail rc(%d)\n", rc);
				break;
			}
			MTEE_DownSemaphore(fork_thread_sema_done);
			rc = SAPU_Log_Dump(VPU1_DUMP|VPU2_DUMP);
			if (rc) {
				ERR_LOG("VPU_DUMP fail rc(%d)\n", rc);
				break;
			}
			break;

		case 102:
			DBG_LOG("SAPU_SDSP_Run_Fake\n");
			DBG_LOG("buf0 offset(0x%x) num(%u)\n", param[0].value.a, param[0].value.b);
			DBG_LOG("prop0 offset(0x%x) size(%u)\n", param[1].value.a, param[1].value.b);
			DBG_LOG("buf1 offset(0x%x) num(%u)\n", param[2].value.a, param[2].value.b);
			DBG_LOG("prop1 offset(0x%x) size(%u)\n", param[2].value.a, param[2].value.b);
			rc = SAPU_SDSP_Run(param[0].value.a, param[0].value.b, param[1].value.a, param[1].value.b,
							   param[2].value.a, param[2].value.b, param[3].value.a, param[3].value.b,
								0, true, NULL);	// true is SW_OP, false is sDSP
			if (rc) {
				DBG_LOG("SAPU_SDSP_Run(0,0) fail rc(%d)\n", rc);
				break;
			}
			DBG_LOG("SAPU_SDSP_Run done\n");
			break;

		case 107:
			DBG_LOG("SAPU_SDSP_Run_Real\n");
			DBG_LOG("buf0 offset(0x%x) num(%u)\n", param[0].value.a, param[0].value.b);
			DBG_LOG("prop0 offset(0x%x) size(%u)\n", param[1].value.a, param[1].value.b);
			DBG_LOG("buf1 offset(0x%x) num(%u)\n", param[2].value.a, param[2].value.b);
			DBG_LOG("prop1 offset(0x%x) size(%u)\n", param[2].value.a, param[2].value.b);
			rc = SAPU_SDSP_BOOTUP();
			if (rc) {
				ERR_LOG("SAPU_SDSP_BOOTUP fail rc(%d)\n", rc);
				break;
			}
			rc = SAPU_SDSP_Run(param[0].value.a, param[0].value.b, param[1].value.a, param[1].value.b,
							   param[2].value.a, param[2].value.b, param[3].value.a, param[3].value.b,
								0, false, NULL);	// true is SW_OP, false is sDSP
			if (rc) {
				DBG_LOG("SAPU_SDSP_Run(0,0) fail rc(%d)\n", rc);
				break;
			}
			//rc = SAPU_Log_Dump(VPU1_DUMP|VPU2_DUMP);
			DBG_LOG("SAPU_SDSP_Run done\n");
			break;

		case 108:
			rc = SAPU_SDSP_Run(0, param[0].value.a, 0, param[0].value.b,
							   0, param[1].value.a, 0, param[1].value.b,
								0, false, NULL);	// true is SW_OP, false is sDSP

			/*
			rc = SAPU_v2_SDSP_Run(param[0].value.a, param[0].value.b, param[1].value.a, param[1].value.b,
								  0, 0, 0, 0,
								  0, false, NULL);	// true is SW_OP, false is sDSP
			*/
			if (rc) {
				DBG_LOG("SAPU_SDSP_Run(0,0) fail rc(%d)\n", rc);
				break;
			}
			break;

		case 109:
			my_reg_info.sdsp1_cmd=0x16a;
			my_reg_info.sdsp1_result=0x0;
			my_reg_info.sdsp2_cmd=0x16a;
			my_reg_info.sdsp2_result=0x0;
			rc = SAPU_SDSP_Run(0, param[0].value.a, 0, param[0].value.b,
							   0, param[1].value.a, 0, param[1].value.b,
								0, false, &my_reg_info);	// true is SW_OP, false is sDSP
			if (!rc) {
				DBG_LOG("vpu1 cmd/result(0x%x/0x%x)\n", my_reg_info.sdsp1_cmd, my_reg_info.sdsp1_result);
				DBG_LOG("vpu2 cmd/result(0x%x/0x%x)\n", my_reg_info.sdsp2_cmd, my_reg_info.sdsp2_result);
			}
			break;

		case 178:
			DBG_LOG("SAPU_SDSP_Run. buf offset(0x%x) num(%u)\n", param[0].value.a, param[0].value.b);
			DBG_LOG("SAPU_SDSP_Run. prop offset(0x%x) size(%u)\n", param[1].value.a, param[1].value.b);
			sdsp1_buf_offset[0] = param[0].value.a;
			sdsp1_buf_num[0] = param[0].value.b;
			sdsp1_prop_offset[0] = param[1].value.a;
			sdsp1_prop_size[0] = param[1].value.b;
			sdsp1_buf_offset[1] = param[2].value.a;
			sdsp1_buf_num[1] = param[2].value.b;
			sdsp1_prop_offset[1] = param[3].value.a;
			sdsp1_prop_size[1] = param[3].value.b;

			/* 1st & 2nd map for VPU0*/
			rc = SAPU_GetTeeMemInfo(&VA1, &VA2, sdsp1_buf_offset[0], sdsp1_buf_num[0], sdsp1_prop_offset[0], sdsp1_prop_size[0], &basePA);
			if(rc){
				ERR_LOG("SAPU_GetTeeMemInfo failed! \n");
				break;
			}

			vpu_buffer = (vpu_buffer_t *)VA1;
			for (i=0 ; i<sdsp1_buf_num[0] ; i++) {
				DBG_LOG("buf[%u](%p)\n", i, &vpu_buffer[i]);
				DBG_LOG("buf[%u]port_id=%u, format=%u\n", i, vpu_buffer[i].port_id, vpu_buffer[i].format);
				DBG_LOG("buf[%u]width=%u, height=%u, plane_count=%u\n", i, vpu_buffer[i].width, vpu_buffer[i].height, vpu_buffer[i].plane_count);
				for (j=0 ; j<vpu_buffer[0].plane_count ; j++) {
					DBG_LOG("buf[%u] plane[%u] stride=%u, length=%u, ptr=0x%llx\n", i, j,
					vpu_buffer[i].planes[j].stride, vpu_buffer[i].planes[j].length, vpu_buffer[i].planes[j].ptr);
				}
			}

			vpu_prop[0] = (vpu_prop_t *)VA2;
			DBG_LOG("prop0(%p) cmd=0x%x, result=%u\n", vpu_prop[0], vpu_prop[0]->cmd, vpu_prop[0]->result);

			/*3rd map for VPU0*/
#if CFG_GZ_SECURE_DSP_WITH_M4U
			rc = MTEE_MmapRegion(vpu_buffer[0].planes[0].ptr-TEE_VPU_SHM_MVA+basePA, (void **)&src, vpu_buffer[0].planes[0].length, MTEE_MAP_USER_DEFAULT);
#else
			rc = MTEE_MmapRegion(vpu_buffer[0].planes[0].ptr, (void **)&src, vpu_buffer[0].planes[0].length, MTEE_MAP_USER_DEFAULT);
#endif
			if (rc) {
				ERR_LOG("%s MTEE_MmapRegion rc fail\n", __func__);
				return rc;
			}
			_src[0] = src;
			_src_size[0] = vpu_buffer[0].planes[0].length;
			DBG_LOG("vpu0 buf[0] planes[0] [12~15]0x%2x 0x%2x 0x%2x 0x%2x\n", src[12], src[13], src[14], src[15]);

			/*4th map for VPU0*/
#if CFG_GZ_SECURE_DSP_WITH_M4U
			rc = MTEE_MmapRegion(vpu_buffer[1].planes[0].ptr-TEE_VPU_SHM_MVA+basePA, (void **)&dst, vpu_buffer[1].planes[0].length, MTEE_MAP_USER_DEFAULT);
#else
			rc = MTEE_MmapRegion(vpu_buffer[1].planes[0].ptr, (void **)&dst, vpu_buffer[1].planes[0].length, MTEE_MAP_USER_DEFAULT);
#endif
			if (rc) {
				ERR_LOG("%s MTEE_MmapRegion rc fail\n", __func__);
				return rc;
			}
			_dst[0] = dst;
			_copysize[0] = vpu_buffer[1].planes[0].length;
			DBG_LOG("vpu0 buf[1] planes[0] [12~15]0x%2x 0x%2x 0x%2x 0x%2x\n", dst[12], dst[13], dst[14], dst[15]);

			rc = MTEE_UnmmapRegion((void *)VA1, sizeof(vpu_buffer_t)*sdsp1_buf_num[0]);
			if(rc)ERR_LOG("MTEE_UnmmapRegion 'sdsp1_buf' failed! \n");
			/*****************for VPU1**********************************/
			/* 1st & 2nd map for VPU0*/
			rc = SAPU_GetTeeMemInfo(&VA1, &VA2, sdsp1_buf_offset[1], sdsp1_buf_num[1], sdsp1_prop_offset[1], sdsp1_prop_size[1], &basePA);
			if(rc){
				ERR_LOG("SAPU_GetTeeMemInfo failed! \n");
				break;
			}

			vpu_buffer = (vpu_buffer_t *)VA1;
			for (i=0 ; i<sdsp1_buf_num[1] ; i++) {
				DBG_LOG("buf[%u](%p)\n", i, &vpu_buffer[i]);
				DBG_LOG("buf[%u]port_id=%u, format=%u\n", i, vpu_buffer[i].port_id, vpu_buffer[i].format);
				DBG_LOG("buf[%u]width=%u, height=%u, plane_count=%u\n", i, vpu_buffer[i].width, vpu_buffer[i].height, vpu_buffer[i].plane_count);
				for (j=0 ; j<vpu_buffer[0].plane_count ; j++) {
					DBG_LOG("buf[%u] plane[%u] stride=%u, length=%u, ptr=0x%llx\n", i, j,
					vpu_buffer[i].planes[j].stride, vpu_buffer[i].planes[j].length, vpu_buffer[i].planes[j].ptr);
				}
			}

			vpu_prop[1] = (vpu_prop_t *)VA2;
			DBG_LOG("prop1(%p) cmd=0x%x, result=%u\n", vpu_prop[1], vpu_prop[1]->cmd, vpu_prop[1]->result);

			/*3rd map for VPU0*/
#if CFG_GZ_SECURE_DSP_WITH_M4U
			rc = MTEE_MmapRegion(vpu_buffer[0].planes[0].ptr-TEE_VPU_SHM_MVA+basePA, (void **)&src, vpu_buffer[0].planes[0].length, MTEE_MAP_USER_DEFAULT);
#else
			rc = MTEE_MmapRegion(vpu_buffer[0].planes[0].ptr, (void **)&src, vpu_buffer[0].planes[0].length, MTEE_MAP_USER_DEFAULT);
#endif
			if (rc) {
				ERR_LOG("%s MTEE_MmapRegion rc fail\n", __func__);
				return rc;
			}
			_src[1] = src;
			_src_size[1] = vpu_buffer[0].planes[0].length;
			DBG_LOG("vpu1 buf[0] planes[0] [12~15]0x%2x 0x%2x 0x%2x 0x%2x\n", src[12], src[13], src[14], src[15]);

			/*4th map for VPU0*/
#if CFG_GZ_SECURE_DSP_WITH_M4U
			rc = MTEE_MmapRegion(vpu_buffer[1].planes[0].ptr-TEE_VPU_SHM_MVA+basePA, (void **)&dst, vpu_buffer[1].planes[0].length, MTEE_MAP_USER_DEFAULT);
#else
			rc = MTEE_MmapRegion(vpu_buffer[1].planes[0].ptr, (void **)&dst, vpu_buffer[1].planes[0].length, MTEE_MAP_USER_DEFAULT);
#endif
			if (rc) {
				ERR_LOG("%s MTEE_MmapRegion rc fail\n", __func__);
				return rc;
			}
			_dst[1] = dst;
			_copysize[1] = vpu_buffer[1].planes[0].length;
			DBG_LOG("vpu1 buf[1] planes[0] [12~15]0x%2x 0x%2x 0x%2x 0x%2x\n", dst[12], dst[13], dst[14], dst[15]);

			rc = MTEE_UnmmapRegion((void *)VA1, sizeof(vpu_buffer_t)*sdsp1_buf_num[1]);
			if(rc)ERR_LOG("MTEE_UnmmapRegion 'sdsp1_buf' failed! \n");

			/*****************for multicore**********************************/

#if 0
			/*start multicore ....*/
			/*multicore sample code, using MTEE_CreateThreadWithCPU*/
			MTEE_CleanDcacheAll();
			DBG_LOG("MTEE_CreateThreadWithCPU start(start)\n");
			rc = MTEE_CreateThreadWithCPU(&helper1_thread_handle, (void *)helper1, NULL, DEFAULT_PRIORITY, 5, 1);
			//DBG_LOG("%s (start)\n", __func__);
			do_my_memcpy(_src[0], _src_size[0], _dst[0], _copysize[0], vpu_prop[0]);
			//DBG_LOG("%s (end), result(%u)\n", __func__, vpu_prop[0]->result);
			MTEE_JoinThread(helper1_thread_handle);
			MTEE_CleanDcacheAll();
			DBG_LOG("MTEE_CreateThreadWithCPU(end)\n");
#endif

#if 1
			MTEE_CleanDcacheAll();
			DBG_LOG("MTEE_UpSemaphoreThread(start)\n");
			MTEE_UpSemaphoreThread(help2_sema_go, 6, 1);
			do_my_memcpy(_src[0], _src_size[0], _dst[0], _copysize[0], vpu_prop[0]);
			MTEE_DownSemaphore(help2_sema_done);
			MTEE_CleanDcacheAll();
			DBG_LOG("MTEE_DownSemaphore(end)\n");
#endif

			MTEE_CleanDcacheAll();
			/*single core ....*/
			DBG_LOG("oneCore %s (start)\n", __func__);
			do_my_memcpy(_src[0], _src_size[0], _dst[0], _copysize[0], vpu_prop[0]);
			do_my_memcpy(_src[1], _src_size[1], _dst[1], _copysize[1], vpu_prop[1]);
			MTEE_CleanDcacheAll();
			DBG_LOG("oneCore %s (end)\n", __func__);

			/*clean cache*/
			MTEE_CleanDcacheAll();

			rc = MTEE_UnmmapRegion(_src[0], _src_size[0]);
			if(rc)ERR_LOG("MTEE_UnmmapRegion '_src[0]' failed! \n");
			rc = MTEE_UnmmapRegion(_dst[0], _copysize[0]);
			if(rc)ERR_LOG("MTEE_UnmmapRegion '_dst[0]' failed! \n");
			rc = MTEE_UnmmapRegion(_src[1], _src_size[1]);
			if(rc)ERR_LOG("MTEE_UnmmapRegion '_src[1]' failed! \n");
			rc = MTEE_UnmmapRegion(_dst[1], _copysize[1]);
			if(rc)ERR_LOG("MTEE_UnmmapRegion '_dst[1]' failed! \n");
			rc = MTEE_UnmmapRegion(vpu_prop[0], sdsp1_prop_size[0]);
			if(rc)ERR_LOG("MTEE_UnmmapRegion 'sdsp1_prop[0]' failed! \n");
			rc = MTEE_UnmmapRegion(vpu_prop[1], sdsp1_prop_size[1]);
			if(rc)ERR_LOG("MTEE_UnmmapRegion 'sdsp1_prop[1]' failed! \n");
			break;

		case 103:
			DBG_LOG("SAPU_Log_Dump!!!!!!!!!!!!!!!!!\n");
			rc = SAPU_Log_Dump(VPU1_DUMP|VPU2_DUMP);
			if (rc) {
				DBG_LOG("SAPU_Log_Dump fail rc(%d)\n", rc);
				break;
			}
			DBG_LOG("SAPU_Log_Dump done\n");
			break;

		case 106:
			rc = SAPU_SDSP_BOOTUP();
			if (rc) {
				DBG_LOG("SAPU_SDSP_UT fail rc(%d)\n", rc);
				break;
			}
			break;

		case 1:
			DBG_LOG("[matrix test] \n");
			rc = MTEE_GetSystemTime(&time1);
			temp64 = test_matrix_64();
			rc = MTEE_GetSystemTime(&time2);
			DBG_LOG("test_matrix_64 end, %d, %lu ms\n", (uint32_t)temp64, (time2.millis-time1.millis)+1000*(time2.seconds-time1.seconds));
			break;

		case 2:
			DBG_LOG("[test_matrix_32] start \n");
			rc = MTEE_GetSystemTime(&time1);
        		temp32 = test_matrix_32();
			rc = MTEE_GetSystemTime(&time2);
        		DBG_LOG("test_matrix_32 end, %d, %lu ms\n", temp32, (time2.millis-time1.millis)+1000*(time2.seconds-time1.seconds));
			break;

		case 3:
			DBG_LOG("[matrix64_to_kernel] \n");
			rc = MTEE_GetSystemTime(&time1);
			temp64 = matrix64_to_kernel();
			rc = MTEE_GetSystemTime(&time2);
			DBG_LOG("[matrix64_to_kernel] end, %d, %lu ms\n", (uint32_t)temp64, (time2.millis-time1.millis)+1000*(time2.seconds-time1.seconds));
			break;

                case 4:
                        DBG_LOG("[matrix32_to_kernel] \n");
                        rc = MTEE_GetSystemTime(&time1);
                        temp32 = matrix32_to_kernel();
                        rc = MTEE_GetSystemTime(&time2);
                        DBG_LOG("[matrix32_to_kernel] end, %d, %lu ms\n", temp32, (time2.millis-time1.millis)+1000*(time2.seconds-time1.seconds));
                        break;

		case TZCMD_TEST_EXIT:
			DBG_LOG("fod_sample TA EXIT !!!!!!!!\n");
			exit(0);
			break;

		case 300:
			if (1) {
				uint8_t *my_va1 = NULL;
				uint32_t my_sz1 = 0;
				uint8_t *my_va2 = NULL;
				uint32_t my_sz2 = 0;

				DBG_LOG("param[0].value.a=0x%x b=0x%x\n", param[0].value.a, param[0].value.b);
				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				if (rc) {
					param[1].value.a = 0x59;
					ERR_LOG("Acquire for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
					break;
				}
				DBG_LOG("Acquire for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
				memset(my_va2, param[0].value.b, my_sz2);

				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				if (rc) {
					param[1].value.a = 0x59;
					ERR_LOG("Acquire for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va1, my_sz1);
					break;
				}
				DBG_LOG("Acquire for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va1, my_sz1);
				memset(my_va1, param[0].value.a, my_sz1);

				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				if (rc) {
					param[1].value.a = 0x59;
					ERR_LOG("Release for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
					break;
				}

				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				if (rc) {
					param[1].value.a = 0x59;
					ERR_LOG("Release for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
					break;
				}
				param[1].value.a = 0x100;
				break;
			}

			if (0) {
				uint8_t *my_va1 = NULL;
				uint32_t my_sz1 = 0;
				uint8_t *my_va2 = NULL;
				uint32_t my_sz2 = 0;
				uint8_t *my_va_retry = NULL;
				uint32_t my_sz_retry = 0;
				int rc;

				MTEE_DumpMteeTeeStaticShm();

				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test?");
				DBG_LOG("Acquire for-ut-test? %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
				MTEE_DumpMteeTeeStaticShm();

				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				DBG_LOG("Acquire for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
				if (!rc) {
					DBG_LOG("0x%x 0x%x 0x%x\n", *(uint32_t *)(my_va2+0), *(uint32_t *)(my_va2+4), *(uint32_t *)(my_va2+8));
				}
				MTEE_DumpMteeTeeStaticShm();

				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				DBG_LOG("Acquire for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va1, my_sz1);
				if (!rc) {
					DBG_LOG("0x%x 0x%x 0x%x\n", *(uint32_t *)(my_va1+0), *(uint32_t *)(my_va1+4), *(uint32_t *)(my_va1+8));
				}
				MTEE_DumpMteeTeeStaticShm();

				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va_retry, &my_sz_retry, "for-ut-test1");
				DBG_LOG("retruy for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va_retry, my_sz_retry);
				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va_retry, &my_sz_retry, "for-ut-test2");
				DBG_LOG("retruy for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va_retry, my_sz_retry);

				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				DBG_LOG("Release for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va1, my_sz1);
				MTEE_DumpMteeTeeStaticShm();
				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				DBG_LOG("Release for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
				MTEE_DumpMteeTeeStaticShm();
				break;
			}


		case 301:
			if (1) {
				uint8_t *my_va1 = NULL;
				uint32_t my_sz1 = 0;
				uint8_t *my_va2 = NULL;
				uint32_t my_sz2 = 0;

				DBG_LOG("param[0].value.a=0x%x b=0x%x\n", param[0].value.a, param[0].value.b);
				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				if (rc) {
					param[1].value.a = 0x59;
					ERR_LOG("Acquire for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
					break;
				}

				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				if (rc) {
					param[1].value.a = 0x59;
					ERR_LOG("Acquire for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va1, my_sz1);
					break;
				}

				for (i=0 ; i<my_sz1 ; i++) {
					if (my_va1[i]!=param[0].value.a) {
						ERR_LOG("va1 needed value=0x%x, but shm[%u]=0x%x\n", param[0].value.a, i, my_va1[i]);
						param[1].value.a = 0x59;
						param[2].value.a = i;
						param[2].value.b = my_va1[i];
						break;
					}
				}

				if (param[1].value.a==0x59) break;
				param[2].value.a = my_sz1;


				for (i=0 ; i<my_sz2 ; i++) {
					if (my_va2[i]!=param[0].value.b) {
						ERR_LOG("va1 needed value=0x%x, but shm[%u]=0x%x\n", param[0].value.b, i, my_va2[i]);
						param[1].value.a = 0x59;
						param[3].value.a = i;
						param[3].value.b = my_va2[i];
						break;
					}
				}
				if (param[1].value.a==0x59) break;
				param[3].value.a = my_sz2;

				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				if (rc) {
					param[1].value.a = 0x59;
					ERR_LOG("Release for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
					break;
				}

				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				if (rc) {
					param[1].value.a = 0x59;
					ERR_LOG("Release for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
					break;
				}
				param[1].value.a = 0x100;
				break;
			}

			if (0) {
				uint8_t *my_va1 = NULL;
				uint32_t my_sz1 = 0;
				uint8_t *my_va2 = NULL;
				uint32_t my_sz2 = 0;
				uint8_t *my_va_retry = NULL;
				uint32_t my_sz_retry = 0;
				int rc;

				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				DBG_LOG("Acquire for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
				if (!rc) {
					memset(my_va2, 0x12, my_sz2);
				}

				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				DBG_LOG("Acquire for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va1, my_sz1);
				if (!rc) {
					memset(my_va1, 0x11, my_sz1);
				}

				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va_retry, &my_sz_retry, "for-ut-test1");
				DBG_LOG("retruy for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va_retry, my_sz_retry);
				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va_retry, &my_sz_retry, "for-ut-test2");
				DBG_LOG("retruy for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va_retry, my_sz_retry);

				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				DBG_LOG("Release for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				DBG_LOG("Release for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va1, my_sz1);
				break;
			}

		case 302:
			if (0) {
				uint8_t *my_va1 = NULL;
				uint32_t my_sz1 = 0;
				uint8_t *my_va2 = NULL;
				uint32_t my_sz2 = 0;
				int rc;


				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				DBG_LOG("Acquire for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va1, my_sz1);
				if (!rc) {
					memset(my_va1, 0x21, my_sz1);
				}

				rc = MTEE_AcquireMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				DBG_LOG("Acquire for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
				if (!rc) {
					memset(my_va2, 0x22, my_sz2);
				}

				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va1, &my_sz1, "for-ut-test1");
				DBG_LOG("Release for-ut-test1 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va1, my_sz1);
				rc = MTEE_ReleaseMteeTeeStaticShm((void *)&my_va2, &my_sz2, "for-ut-test2");
				DBG_LOG("Release for-ut-test2 %s, va(%p), sz(%u)\n", rc==0?"done":"fail", my_va2, my_sz2);
			}
			break;

		default:
			DBG_LOG("%s unknown(%d)\n", __func__, command);
			break;
	}

	return 0;
}


// IPC message buffer
static uint8_t msg_buf[MAX_PORT_BUF_SIZE];

// service states
static struct tipc_srv_state _srv_states[countof(_services)] = {
	[0 ... (countof(_services) - 1)] = {
		.port = INVALID_IPC_HANDLE,
	},
};

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

	/* handle events */
	while (1) {
		uevent.handle = INVALID_IPC_HANDLE;
		uevent.event  = 0;
		uevent.cookie = NULL;
		rc = wait_any(&uevent, -1);
		if (rc < 0) {
			ERR_LOG("wait_any failed (%d)", rc);
			continue;
		}
		if (rc == NO_ERROR) { /* got an event */
			dispatch_event(&uevent, msg_buf, sizeof(msg_buf));
		}
		if (need_exit) {
			need_exit = 0;
			break;
		}
	}

	/* Terminate service (DO NOT MODIFY) */
	kill_services(_services, countof(_services), _srv_states);
	return 0;
}
