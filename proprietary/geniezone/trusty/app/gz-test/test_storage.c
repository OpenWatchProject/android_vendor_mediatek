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
#include <lib/storage/storage.h>
#include <mtee_server.h>
#include <tz_private/sys_ipc.h>
#include <lib/mtee/mtee_srv.h>

#define LOG_TAG "STORAGE"

#include "gz-test_common.h"

void test_storage_bypass(int command)
{
	switch (command) {
		case 200:
			if (1) {
				#define MYSIZE 4096
				uint8_t TA_TAs_sec_va[MYSIZE];
				uint32_t TA_TAs_sec_size = MYSIZE;
				memset(TA_TAs_sec_va, 0x36, TA_TAs_sec_size);
				DBG_LOG("Write Patten 0x%02x at stack mem\n", 0x36);

				storage_session_t ss;
				file_handle_t handle;
				int rc = storage_open_session(&ss, STORAGE_CLIENT_TD_PORT);
				if (rc < 0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
					break;
				}
				rc = storage_open_file(ss, &handle, "mydata01",
						   STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
				if (rc!=0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
				}
				rc = storage_write(handle, 0, TA_TAs_sec_va, TA_TAs_sec_size, STORAGE_OP_COMPLETE);
				DBG_LOG("storage_write return size=%d\n", rc);

				storage_close_file(handle);
				storage_close_session(ss);
			}
			break;
		case 201:
			if (1) {
				#define MYSIZE 4096
				uint8_t *TA_TAs_sec_va;
				uint32_t TA_TAs_sec_size = MYSIZE;
				TA_TAs_sec_va = (uint8_t *)malloc(TA_TAs_sec_size);
				if (TA_TAs_sec_va==NULL) {
					DBG_LOG("malloc mem fail %d\n", __LINE__);
					break;
				}
				memset(TA_TAs_sec_va, 0x37, TA_TAs_sec_size);
				DBG_LOG("Write Patten 0x%02x at malloc mem\n", 0x37);

				storage_session_t ss;
				file_handle_t handle;
				int rc = storage_open_session(&ss, STORAGE_CLIENT_TD_PORT);
				if (rc < 0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
					break;
				}
				rc = storage_open_file(ss, &handle, "mydata01",
						   STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
				if (rc!=0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
				}
				rc = storage_write(handle, 0, TA_TAs_sec_va, TA_TAs_sec_size, STORAGE_OP_COMPLETE);
				DBG_LOG("storage_write return size=%d\n", rc);

				storage_close_file(handle);
				storage_close_session(ss);
				free(TA_TAs_sec_va);
			}
			break;
		case 202:
			if (1) {
				TZ_RESULT tz_ret;
				MTEE_MEM_PARAM TAs_sec_info;
				uint8_t *TA_TAs_sec_va;
				uint32_t TA_TAs_sec_size;
				MTEE_SECUREMEM_HANDLE TA_TAs_sec_handle = 0;

				// Create buf between TAs, just get handle
				TA_Mem_AllocMem(&TA_TAs_sec_handle, 4096, 4096);

				// Query (buf between TAs) va
				tz_ret = TA_Mem_QueryMem(TA_TAs_sec_handle, &TAs_sec_info);
				if(tz_ret != TZ_RESULT_SUCCESS) {
					break;
				}
				TA_TAs_sec_va = (uint8_t *)TAs_sec_info.buffer;
				TA_TAs_sec_size = (uint32_t)TAs_sec_info.size;
				memset(TA_TAs_sec_va, 0x38, TA_TAs_sec_size);
				DBG_LOG("Write Patten 0x%02x at TA_Mem_QueryMem mem\n", 0x38);

				storage_session_t ss;
				file_handle_t handle;
				int rc = storage_open_session(&ss, STORAGE_CLIENT_TD_PORT);
				if (rc < 0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
					break;
				}
				rc = storage_open_file(ss, &handle, "mydata01",
						   STORAGE_FILE_OPEN_CREATE | STORAGE_FILE_OPEN_TRUNCATE,
                           STORAGE_OP_COMPLETE);
				if (rc!=0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
				}
				rc = storage_write(handle, 0, TA_TAs_sec_va, TA_TAs_sec_size, STORAGE_OP_COMPLETE);
				DBG_LOG("storage_write return size=%d\n", rc);

				storage_close_file(handle);
				storage_close_session(ss);

				rc = TA_Mem_UnreferenceMem(TA_TAs_sec_handle);
				if (rc!=0) {
					DBG_LOG("TA_Mem_UnreferenceMem ret =%d %d\n", rc, __LINE__);
				}
			}
			break;
		case 300:
			if (1) {
				#define MYSIZE 4096
				uint8_t TA_TAs_sec_va[MYSIZE];
				uint32_t TA_TAs_sec_size = MYSIZE;
				DBG_LOG("mem by stack\n");
				memset(TA_TAs_sec_va, 0xcc, TA_TAs_sec_size);

				storage_session_t ss;
				file_handle_t handle;
				int rc = storage_open_session(&ss, STORAGE_CLIENT_TD_PORT);
				if (rc < 0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
					break;
				}
				rc = storage_open_file(ss, &handle, "mydata01",
						   0,
                           STORAGE_OP_COMPLETE);
				if (rc!=0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
				}
				rc = storage_read(handle, 0, TA_TAs_sec_va, TA_TAs_sec_size);
				if (rc!=(int)TA_TAs_sec_size) {
					DBG_LOG("faild storage_read return size=%d not %d\n", rc, TA_TAs_sec_size);
				}
				if (1) {
					uint32_t i;
					uint32_t error_cnt=0;
					for (i=0 ; i<TA_TAs_sec_size; i++) {
						if ( TA_TAs_sec_va[i]>0x38 || TA_TAs_sec_va[i]<0x36 ) {
							DBG_LOG("index %u not 0x38~0x36, 0x%2x\n", i, TA_TAs_sec_va[i]);
							error_cnt++;
						}
					}
					if (error_cnt==0)
						DBG_LOG("Read Patten 0x%02x\n", TA_TAs_sec_va[TA_TAs_sec_size-1]);
				}

				storage_close_file(handle);
				storage_close_session(ss);
			}
			break;
		case 301:
			if (1) {
				#define MYSIZE 4096
				uint8_t *TA_TAs_sec_va;
				uint32_t TA_TAs_sec_size = MYSIZE;
				TA_TAs_sec_va = (uint8_t *)malloc(TA_TAs_sec_size);
				if (TA_TAs_sec_va==NULL) {
					DBG_LOG("malloc mem fail %d\n", __LINE__);
					break;
				}
				DBG_LOG("mem by malloc\n");
				memset(TA_TAs_sec_va, 0xcc, TA_TAs_sec_size);

				storage_session_t ss;
				file_handle_t handle;
				int rc = storage_open_session(&ss, STORAGE_CLIENT_TD_PORT);
				if (rc < 0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
					break;
				}
				rc = storage_open_file(ss, &handle, "mydata01",
						   0,
                           STORAGE_OP_COMPLETE);
				if (rc!=0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
				}
				rc = storage_read(handle, 0, TA_TAs_sec_va, TA_TAs_sec_size);
				if (rc!=(int)TA_TAs_sec_size) {
					DBG_LOG("faild storage_read return size=%d not %d\n", rc, TA_TAs_sec_size);
				}
				if (1) {
					uint32_t i;
					uint32_t error_cnt=0;
					for (i=0 ; i<TA_TAs_sec_size; i++) {
						if ( TA_TAs_sec_va[i]>0x38 || TA_TAs_sec_va[i]<0x36 ) {
							DBG_LOG("index %u not 0x38~0x36, 0x%2x\n", i, TA_TAs_sec_va[i]);
							error_cnt++;
						}
					}
					if (error_cnt==0)
						DBG_LOG("Read Patten 0x%02x\n", TA_TAs_sec_va[TA_TAs_sec_size-1]);
				}

				storage_close_file(handle);
				storage_close_session(ss);
				free(TA_TAs_sec_va);
			}
			break;
		case 302:
			if (1) {
				TZ_RESULT tz_ret;
				MTEE_MEM_PARAM TAs_sec_info;
				uint8_t *TA_TAs_sec_va;
				uint32_t TA_TAs_sec_size;
				MTEE_SECUREMEM_HANDLE TA_TAs_sec_handle = 0;

				// Create buf between TAs, just get handle
				TA_Mem_AllocMem(&TA_TAs_sec_handle, 4096, 4096);

				// Query (buf between TAs) va
				tz_ret = TA_Mem_QueryMem(TA_TAs_sec_handle, &TAs_sec_info);
				if(tz_ret != TZ_RESULT_SUCCESS) {
					break;
				}
				TA_TAs_sec_va = (uint8_t *)TAs_sec_info.buffer;
				TA_TAs_sec_size = (uint32_t)TAs_sec_info.size;
				DBG_LOG("mem by TA_Mem_AllocMem\n");
				memset(TA_TAs_sec_va, 0xcc, TA_TAs_sec_size);

				storage_session_t ss;
				file_handle_t handle;
				int rc = storage_open_session(&ss, STORAGE_CLIENT_TD_PORT);
				if (rc < 0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
					break;
				}
				rc = storage_open_file(ss, &handle, "mydata01",
						   0,
                           STORAGE_OP_COMPLETE);
				if (rc!=0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
				}
				rc = storage_read(handle, 0, TA_TAs_sec_va, TA_TAs_sec_size);
				if (rc!=(int)TA_TAs_sec_size) {
					DBG_LOG("faild storage_read return size=%d not %d\n", rc, TA_TAs_sec_size);
				}
				if (1) {
					uint32_t i;
					uint32_t error_cnt=0;
					for (i=0 ; i<TA_TAs_sec_size; i++) {
						if ( TA_TAs_sec_va[i]>0x38 || TA_TAs_sec_va[i]<0x36 ) {
							DBG_LOG("index %u not 0x38~0x36, 0x%2x\n", i, TA_TAs_sec_va[i]);
							error_cnt++;
						}
					}
					if (error_cnt==0)
						DBG_LOG("Read Patten 0x%02x\n", TA_TAs_sec_va[TA_TAs_sec_size-1]);
				}

				storage_close_file(handle);
				storage_close_session(ss);

				rc = TA_Mem_UnreferenceMem(TA_TAs_sec_handle);
				if (rc!=0) {
					DBG_LOG("TA_Mem_UnreferenceMem ret =%d %d\n", rc, __LINE__);
				}
			}
			break;
		case 310:
			if (1) {
				#define MYSIZE 4096
				uint8_t TA_TAs_sec_va[MYSIZE];
				uint32_t TA_TAs_sec_size = MYSIZE;
				DBG_LOG("rpmb mem by stack\n");
				memset(TA_TAs_sec_va, 0xcc, TA_TAs_sec_size);

				storage_session_t ss;
				int rc = storage_open_session(&ss, STORAGE_CLIENT_TD_PORT);
				if (rc < 0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
					break;
				}
				rc = storage_rpmb_read(ss, 0, MYSIZE/256, TA_TAs_sec_va, TA_TAs_sec_size);
				DBG_LOG("rpmb 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
					TA_TAs_sec_va[0], TA_TAs_sec_va[1], TA_TAs_sec_va[2], TA_TAs_sec_va[1024], TA_TAs_sec_va[2048]);
				storage_close_session(ss);
			}
			break;
		case 311:
			if (1) {
				#define MYSIZE 4096
				uint8_t *TA_TAs_sec_va;
				uint32_t TA_TAs_sec_size = MYSIZE;
				TA_TAs_sec_va = (uint8_t *)malloc(TA_TAs_sec_size);
				if (TA_TAs_sec_va==NULL) {
					DBG_LOG("malloc mem fail %d\n", __LINE__);
					break;
				}
				DBG_LOG("rpmb mem by malloc\n");
				memset(TA_TAs_sec_va, 0xcc, TA_TAs_sec_size);

				storage_session_t ss;
				int rc = storage_open_session(&ss, STORAGE_CLIENT_TD_PORT);
				if (rc < 0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
					break;
				}
				rc = storage_rpmb_read(ss, 0, MYSIZE/256, TA_TAs_sec_va, TA_TAs_sec_size);
				DBG_LOG("rpmb 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
					TA_TAs_sec_va[0], TA_TAs_sec_va[1], TA_TAs_sec_va[2], TA_TAs_sec_va[1024], TA_TAs_sec_va[2048]);
				storage_close_session(ss);
				free(TA_TAs_sec_va);
			}
			break;
		case 312:
			if (1) {
				TZ_RESULT tz_ret;
				MTEE_MEM_PARAM TAs_sec_info;
				uint8_t *TA_TAs_sec_va;
				uint32_t TA_TAs_sec_size;
				MTEE_SECUREMEM_HANDLE TA_TAs_sec_handle = 0;

				// Create buf between TAs, just get handle
				TA_Mem_AllocMem(&TA_TAs_sec_handle, 4096, 4096);

				// Query (buf between TAs) va
				tz_ret = TA_Mem_QueryMem(TA_TAs_sec_handle, &TAs_sec_info);
				if(tz_ret != TZ_RESULT_SUCCESS) {
					break;
				}
				TA_TAs_sec_va = (uint8_t *)TAs_sec_info.buffer;
				TA_TAs_sec_size = (uint32_t)TAs_sec_info.size;
				DBG_LOG("rpmb mem by TA_Mem_AllocMem\n");
				memset(TA_TAs_sec_va, 0xcc, TA_TAs_sec_size);

				storage_session_t ss;
				int rc = storage_open_session(&ss, STORAGE_CLIENT_TD_PORT);
				if (rc < 0) {
					DBG_LOG("failed (%d) to open session %d\n", rc, __LINE__);
					break;
				}
				rc = storage_rpmb_read(ss, 0, MYSIZE/256, TA_TAs_sec_va, TA_TAs_sec_size);
				DBG_LOG("rpmb 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
					TA_TAs_sec_va[0], TA_TAs_sec_va[1], TA_TAs_sec_va[2], TA_TAs_sec_va[1024], TA_TAs_sec_va[2048]);
				storage_close_session(ss);

				rc = TA_Mem_UnreferenceMem(TA_TAs_sec_handle);
				if (rc!=0) {
					DBG_LOG("TA_Mem_UnreferenceMem ret =%d %d\n", rc, __LINE__);
				}
			}
			break;
		default:
			DBG_LOG("%s not support %d cmd\n", __func__, command);
			break;
	}

	return;
}


